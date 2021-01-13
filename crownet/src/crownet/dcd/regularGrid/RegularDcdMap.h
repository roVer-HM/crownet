/*
 * Regular_dcdMap.h
 *
 *  Created on: Nov 24, 2020
 *      Author: sts
 */

#pragma once

#include "crownet/dcd/generic/Cell.h"
#include "crownet/dcd/generic/DcdMap.h"

namespace crownet {

using RegularDcdMap = DcDMap<GridCellID, IntIdentifer, omnetpp::simtime_t>;

class RegularDcdMapFactory {
 public:
  RegularDcdMapFactory(std::pair<double, double> gridSize,
                       std::pair<int, int> gridDim)
      : gridSize(gridSize), gridDim(gridDim) {}

  RegularDcdMap create(const IntIdentifer& ownerID);
  std::shared_ptr<RegularDcdMap> create_shared_ptr(const IntIdentifer& ownerID);

 private:
  std::pair<double, double> gridSize;
  std::pair<int, int> gridDim;
};

}  // namespace crownet
