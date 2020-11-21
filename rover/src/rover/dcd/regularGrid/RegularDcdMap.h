/*
 * Regular_dcdMap.h
 *
 *  Created on: Nov 24, 2020
 *      Author: sts
 */

#pragma once

#include "rover/dcd/generic/Cell.h"
#include "rover/dcd/generic/DcdMap.h"

namespace rover {

using RegularDcdMap = DcDMap<GridCellID, IntIdentifer, omnetpp::simtime_t>;

class RegularDcdMapFactory {
 public:
  RegularDcdMapFactory(std::pair<double, double> gridSize,
                       std::pair<int, int> gridDim)
      : gridSize(gridSize), gridDim(gridDim) {}

  RegularDcdMap create(const IntIdentifer& ownerID);

 private:
  std::pair<double, double> gridSize;
  std::pair<int, int> gridDim;
};

}  // namespace rover
