/*
 * CellKeyProvider.h
 *
 *  Created on: Nov 30, 2020
 *      Author: sts
 */

#pragma once

#include "inet/common/geometry/Geometry_m.h"
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
  GridCellIDKeyProvider(const inet::Coord& cellSize,
                        const inet::Coord& gridSize)
      : cellSize(cellSize), gridSize(gridSize) {}

  virtual GridCellID getCellKey(const traci::TraCIPosition& pos) override;

  inet::Coord  getCellSize() const { return cellSize; }
  inet::Coord  getGridSize() const { return gridSize; }

 private:
  inet::Coord cellSize;
  inet::Coord gridSize;
};

}  // namespace crownet
