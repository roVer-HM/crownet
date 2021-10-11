/*
 * CellKeyProvider.h
 *
 *  Created on: Nov 30, 2020
 *      Author: sts
 */

#pragma once

#include "crownet/dcd/generic/Cell.h"
#include "traci/Position.h"

namespace crownet {

template <typename C>
class CellKeyProvider {
 public:
  virtual ~CellKeyProvider() = default;

  virtual C getCellKey(const traci::TraCIPosition& pos) = 0;
};

class GridCellIDKeyProvider : public CellKeyProvider<GridCellID> {
 public:
  GridCellIDKeyProvider(std::pair<double, double> cellSize,
                        std::pair<int, int> gridDim)
      : cellSize(cellSize), gridDim(gridDim) {}

  virtual GridCellID getCellKey(const traci::TraCIPosition& pos) override;

  std::pair<double, double> getcellSize() const { return cellSize; }
  std::pair<int, int> getGridDim() const { return gridDim; }

 private:
  std::pair<double, double> cellSize;
  std::pair<int, int> gridDim;
};

}  // namespace crownet
