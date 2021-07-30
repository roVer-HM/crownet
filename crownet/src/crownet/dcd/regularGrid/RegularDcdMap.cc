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

RegularDcdMapFactory::RegularDcdMapFactory(std::pair<double, double> gridSize,
                                             std::pair<int, int> gridDim)
        : gridSize(gridSize),
          gridDim(gridDim),
          timeProvider(std::make_shared<SimTimeProvider>()) {

    visitor_dispatcher["ymf"] = [this](){return std::make_shared<YmfVisitor>(timeProvider->now());};
    visitor_dispatcher["mean"] = [this](){return std::make_shared<MeanVisitor>(timeProvider->now());};
    visitor_dispatcher["median"] = [this](){return std::make_shared<MedianVisitor>(timeProvider->now());};
    visitor_dispatcher["invSourceDist"] = [this](){return std::make_shared<InvSourceDistVisitor>(timeProvider->now());};

}


RegularDcdMap RegularDcdMapFactory::create(const IntIdentifer& ownerID) {
  std::shared_ptr<GridCellIDKeyProvider> provider =
      std::make_shared<GridCellIDKeyProvider>(gridSize, gridDim);
  return RegularDcdMap(ownerID, provider, timeProvider);
}

std::shared_ptr<RegularDcdMap> RegularDcdMapFactory::create_shared_ptr(
    const IntIdentifer& ownerID) {
  std::shared_ptr<GridCellIDKeyProvider> provider =
      std::make_shared<GridCellIDKeyProvider>(gridSize, gridDim);
  return std::make_shared<RegularDcdMap>(ownerID, provider, timeProvider);
}

std::shared_ptr<GridCellDistance> RegularDcdMapFactory::createDistanceProvider(){
    return std::make_shared<GridCellDistance>(gridDim);
}

std::shared_ptr<TimestampedGetEntryVisitor<RegularCell>> RegularDcdMapFactory::createValueVisitor(const std::string& mapType){
    if (visitor_dispatcher.find(mapType) == visitor_dispatcher.end()){
        throw cRuntimeError("No visitor defined for mapType %s", mapType.c_str());
    }
    return visitor_dispatcher[mapType]();
}

}  // namespace crownet
