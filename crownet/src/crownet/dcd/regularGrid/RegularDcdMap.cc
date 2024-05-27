/*
 * RegularDcdMap.cc
 *
 *  Created on: Nov 30, 2020
 *      Author: sts
 */

#include "crownet/dcd/regularGrid/RegularDcdMap.h"
#include "crownet/dcd/regularGrid/MapCellAggregationAlgorithms.h"
#include "crownet/dcd/identifier/CellKeyProvider.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/common/util/FileWriter.h"

#include "crownet/dcd/generic/transmissionOrderStrategies/InsertionOrderedCellIdStream.h"
#include "crownet/dcd/generic/transmissionOrderStrategies/OrderByCellIdStream.h"
#include "crownet/dcd/generic/transmissionOrderStrategies/OrderdCellIdVectorProvider.h"



namespace crownet {

//RegularDcdMapFactory::RegularDcdMapFactory(const RegularGridInfo& grid)
RegularDcdMapFactory::RegularDcdMapFactory(std::shared_ptr<OsgCoordinateConverter> converter)
        : grid(converter->getGridDescription()),
          converter(converter),
          cellKeyProvider(std::make_shared<GridCellIDKeyProvider>(converter)),
          timeProvider(std::make_shared<SimTimeProvider>()) {

    visitor_dispatcher["ymf"] = [this](MapCfg* mapCfg){
        return std::make_shared<YmfVisitor>(timeProvider->now());
    };
    visitor_dispatcher["ymfPlusDist"] = [this](MapCfg* mapCfg){
        double alpha = check_and_cast<MapCfgYmfPlusDist*>(mapCfg)->getAlpha();
        return std::make_shared<YmfPlusDistVisitor>(alpha, timeProvider->now());
    };
    visitor_dispatcher["ymfPlusDistStep"] = [this](MapCfg* mapCfg){
        double alpha = check_and_cast<MapCfgYmfPlusDistStep*>(mapCfg)->getAlpha();
        double stepDist = check_and_cast<MapCfgYmfPlusDistStep*>(mapCfg)->getStepDist();

        return std::make_shared<YmfPlusDistStepVisitor>(alpha, timeProvider->now(), stepDist);
    };
    visitor_dispatcher["mean"] = [this](MapCfg* mapCfg){
        return std::make_shared<MeanVisitor>(timeProvider->now());
    };
    visitor_dispatcher["median"] = [this](MapCfg* mapCfg){
        return std::make_shared<MedianVisitor>(timeProvider->now());
    };
    visitor_dispatcher["invSourceDist"] = [this](MapCfg* mapCfg){
        return std::make_shared<InvSourceDistVisitor>(timeProvider->now());
    };
    visitor_dispatcher["local"] = [this](MapCfg* mapCfg){
        return std::make_shared<LocalSelector>(timeProvider->now());
    };

    cellIdStream_dispatcher["default"] = [](){
        return std::make_shared<InsertionOrderedCellIdStream<GridCellID, IntIdentifer, omnetpp::simtime_t>>();
    };
    cellIdStream_dispatcher["insertionOrder"] = [](){
        return std::make_shared<InsertionOrderedCellIdStream<GridCellID, IntIdentifer, omnetpp::simtime_t>>();
    };
    cellIdStream_dispatcher["orderBySentLastAsc_DistAsc"] = [](){
        bool lastSentAscendingOrder = true;
        bool distanceAscendingOrder = true;
        using vecProvider_t = OrderByLastSentAndCellDistance<GridCellID, IntIdentifer, omnetpp::simtime_t>;
        auto vecProvider = std::make_shared<vecProvider_t>(lastSentAscendingOrder, distanceAscendingOrder);
        return std::make_shared<OrderByCellIdStream<GridCellID, IntIdentifer, omnetpp::simtime_t>>(vecProvider);
    };

    cellIdStream_dispatcher["orderByDistanceAscending"] = [](){
        bool ascending = true;
        using vecProvider_t = OrderByCellDistance<GridCellID, IntIdentifer, omnetpp::simtime_t>;
        auto vecProvider = std::make_shared<vecProvider_t>(ascending);
        return std::make_shared<OrderByCellIdStream<GridCellID, IntIdentifer, omnetpp::simtime_t>>(vecProvider);
    };

    cellIdStream_dispatcher["orderByDistanceDescending"] = [](){
        bool ascending = false;
        using vecProvider_t = OrderByCellDistance<GridCellID, IntIdentifer, omnetpp::simtime_t>;
        auto vecProvider = std::make_shared<vecProvider_t>(ascending);
        return std::make_shared<OrderByCellIdStream<GridCellID, IntIdentifer, omnetpp::simtime_t>>(vecProvider);
    };


    cellIdStream_dispatcher["orderBySentLastAsc_DistDesc"] = [](){
        bool lastSentAscendingOrder = true;
        bool distanceAscendingOrder = false; // descending order (5,4,3,2,1)
        using vecProvider_t = OrderByLastSentAndCellDistance<GridCellID, IntIdentifer, omnetpp::simtime_t>;
        auto vecProvider = std::make_shared<vecProvider_t>(lastSentAscendingOrder, distanceAscendingOrder);
        return std::make_shared<OrderByCellIdStream<GridCellID, IntIdentifer, omnetpp::simtime_t>>(vecProvider);
    };

}


RegularDcdMap RegularDcdMapFactory::create(const IntIdentifer& ownerID, const std::string& idStreamType) {
  auto streamer = createCellIdStream(idStreamType); // create new one do not share
  return RegularDcdMap(ownerID, cellKeyProvider, timeProvider, streamer);
}

std::shared_ptr<RegularDcdMap> RegularDcdMapFactory::create_shared_ptr(
    const IntIdentifer& ownerID, const std::string& idStreamType) {
  auto streamer = createCellIdStream(idStreamType); // create new one do not share
  return std::make_shared<RegularDcdMap>(ownerID, cellKeyProvider, timeProvider, streamer);
}

std::shared_ptr<CellAggregationAlgorihm<RegularCell>> RegularDcdMapFactory::createValueVisitor(MapCfg* mapCfg){

    auto mapType = mapCfg->getMapType();
    if (visitor_dispatcher.find(mapType) == visitor_dispatcher.end()){
        throw cRuntimeError("No visitor defined for mapType %s", mapType);
    }
    return visitor_dispatcher[mapType](mapCfg);
}

std::shared_ptr<ICellIdStream<GridCellID, IntIdentifer, omnetpp::simtime_t>> RegularDcdMapFactory::createCellIdStream(const std::string& typeName){
    if (cellIdStream_dispatcher.find(typeName) == cellIdStream_dispatcher.end()){
        throw cRuntimeError("No CellIdStream defined for type %s", typeName.c_str());
    }
    return cellIdStream_dispatcher[typeName]();
}
void RegularDcdMapFactory::create_result_db(std::string path, int maxCommitCount){
    sqlApi = std::make_shared<SqlLiteApi>(maxCommitCount); //max commit count
    auto absPath = BaseFileWriter::getAbsOutputPath(path, ".db");
    sqlApi->initialize(absPath.c_str());
}

bool RegularDcdMapFactory::use_result_db() const {
    return sqlApi != nullptr;
}


std::shared_ptr<SqlLiteApi> RegularDcdMapFactory::getSqlApi(){
    if (sqlApi == nullptr){
        throw cRuntimeError("SqlApi not configured in RegularDcdMapFactory");
    }
    return sqlApi;
}

}  // namespace crownet
