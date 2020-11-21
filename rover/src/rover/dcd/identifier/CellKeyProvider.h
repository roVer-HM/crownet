/*
 * CellKeyProvider.h
 *
 *  Created on: Nov 30, 2020
 *      Author: sts
 */

#pragma once

#include "rover/dcd/generic/Cell.h"
#include "traci/Position.h"

namespace rover {

template <typename C>
class CellKeyProvider {
 public:
  virtual ~CellKeyProvider() = default;

  virtual C getCellKey(const traci::TraCIPosition& pos) = 0;
};

class GridCellIDKeyProvider : public CellKeyProvider<GridCellID> {
 public:
  GridCellIDKeyProvider(std::pair<double, double> gridSize,
                        std::pair<int, int> gridDim)
      : gridSize(gridSize), gridDim(gridDim) {}

  virtual GridCellID getCellKey(const traci::TraCIPosition& pos) override;

 private:
  std::pair<double, double> gridSize;
  std::pair<int, int> gridDim;
};

}  // namespace rover
