/*
 * Regular_dcdMap.h
 *
 *  Created on: Nov 24, 2020
 *      Author: sts
 */

#pragma once

#include "crownet/dcd/generic/Cell.h"
#include "crownet/dcd/generic/DcdMap.h"
#include "crownet/dcd/generic/DcdMapWatcher.h"
#include "crownet/dcd/regularGrid/DistanceProvider.h"
#include "crownet/dcd/regularGrid/RegularCell.h"
#include <functional>

namespace crownet {

using RegularDcdMap = DcDMap<GridCellID, IntIdentifer, omnetpp::simtime_t>;
using RegularDcdMapWatcher = DcdMapWatcher<GridCellID, IntIdentifer, omnetpp::simtime_t>;
using VisitorCreator = std::function<std::shared_ptr<TimestampedGetEntryVisitor<RegularCell>>()>;



class RegularDcdMapFactory {
 public:
  RegularDcdMapFactory(std::pair<double, double> gridSize,
                       std::pair<int, int> gridDim);

  std::map<std::string, VisitorCreator>visitor_dispatcher;

  RegularDcdMap create(const IntIdentifer& ownerID);
  std::shared_ptr<RegularDcdMap> create_shared_ptr(const IntIdentifer& ownerID);
  std::shared_ptr<GridCellDistance> createDistanceProvider();
  std::shared_ptr<TimestampedGetEntryVisitor<RegularCell>> createValueVisitor(const std::string& mapType);


 private:
  std::pair<double, double> gridSize;
  std::pair<int, int> gridDim;
  std::shared_ptr<SimTimeProvider> timeProvider;

};

}  // namespace crownet
