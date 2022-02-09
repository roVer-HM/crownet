/*
 * RegularDcdMap.cc
 *
 *  Created on: Nov 30, 2020
 *      Author: sts
 */

#include "crownet/dcd/regularGrid/RegularDcdMap.h"
#include "crownet/dcd/identifier/CellKeyProvider.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"

namespace crownet {

RegularDcdMapFactory::RegularDcdMapFactory(const RegularGridInfo& grid)
        : grid(grid),
          cellKeyProvider(std::make_shared<GridCellIDKeyProvider>(grid)),
          timeProvider(std::make_shared<SimTimeProvider>()) {

    visitor_dispatcher["ymf"] = [this](){return std::make_shared<YmfVisitor>(timeProvider->now());};
    visitor_dispatcher["mean"] = [this](){return std::make_shared<MeanVisitor>(timeProvider->now());};
    visitor_dispatcher["median"] = [this](){return std::make_shared<MedianVisitor>(timeProvider->now());};
    visitor_dispatcher["invSourceDist"] = [this](){return std::make_shared<InvSourceDistVisitor>(timeProvider->now());};
    visitor_dispatcher["local"] = [this](){return std::make_shared<LocalSelector>(timeProvider->now());};

    cellIdStream_dispatcher["default"] = [](){return std::make_shared<InsertionOrderedCellIdStream<GridCellID, IntIdentifer, omnetpp::simtime_t>>();};
    cellIdStream_dispatcher["insertionOrder"] = [](){return std::make_shared<InsertionOrderedCellIdStream<GridCellID, IntIdentifer, omnetpp::simtime_t>>();};
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

std::shared_ptr<TimestampedGetEntryVisitor<RegularCell>> RegularDcdMapFactory::createValueVisitor(const std::string& mapType){
    if (visitor_dispatcher.find(mapType) == visitor_dispatcher.end()){
        throw cRuntimeError("No visitor defined for mapType %s", mapType.c_str());
    }
    return visitor_dispatcher[mapType]();
}

std::shared_ptr<ICellIdStream<GridCellID, IntIdentifer, omnetpp::simtime_t>> RegularDcdMapFactory::createCellIdStream(const std::string& typeName){
    if (cellIdStream_dispatcher.find(typeName) == cellIdStream_dispatcher.end()){
        throw cRuntimeError("No CellIdStream defined for type %s", typeName.c_str());
    }
    return cellIdStream_dispatcher[typeName]();
}

}  // namespace crownet
