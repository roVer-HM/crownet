/*
 * GridDensityMap.h
 *
 *  Created on: Aug 25, 2020
 *      Author: sts
 */

#pragma once

#include <math.h>
#include <omnetpp/simtime_t.h>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/map.hpp>
#include <map>
#include <unordered_map>
#include "inet/common/geometry/common/Coord.h"
#include "rover/common/positionMap/DensityMeasure.h"
#include "rover/common/positionMap/PositionMap.h"

namespace rover {

template <typename CELL_ID, typename NODE_ID>
class RegularGridMap
    : public PositionMap<CELL_ID, CellEntry<DensityMeasure<NODE_ID>,
                                            DensityMeasureCtor<NODE_ID>>> {
 public:
  using CellId = CELL_ID;
  using NodeId = NODE_ID;
  RegularGridMap(NodeId id, double gridSize);
  virtual ~RegularGridMap() = default;

  using PositionMap<CELL_ID,
                    CellEntry<DensityMeasure<NODE_ID>,
                              DensityMeasureCtor<NODE_ID>>>::incrementLocal;
  virtual void incrementLocal(const inet::Coord& coord, const NodeId nodeId,
                              const omnetpp::simtime_t& t,
                              bool ownPosition = false);

 private:
  double gridSize;
};

/// implementation RegularGridMap<CELL_ID, NODE_ID>
template <typename CELL_ID, typename NODE_ID>
inline RegularGridMap<CELL_ID, NODE_ID>::RegularGridMap(NodeId id,
                                                        double gridSize)
    : PositionMap<CELL_ID, CellEntry<DensityMeasure<NODE_ID>,
                                     DensityMeasureCtor<NODE_ID>>>(id),
      gridSize(gridSize) {}

template <typename CELL_ID, typename NODE_ID>
inline void RegularGridMap<CELL_ID, NODE_ID>::incrementLocal(
    const inet::Coord& coord, const NodeId nodeId, const omnetpp::simtime_t& t,
    bool ownPosition) {
  CellId cellId =
      std::make_pair(floor(coord.x / gridSize), floor(coord.y / gridSize));

  std::shared_ptr<LocalDensityMeasure<NODE_ID>> locMeasure =
      std::static_pointer_cast<LocalDensityMeasure<NODE_ID>>(
          this->getCellEntry(cellId).getLocal());
  locMeasure->incrementCount(t);
  auto ret = locMeasure->nodeIds.insert(nodeId);
  if (!ret.second) {
    throw omnetpp::cRuntimeError("duplicate node in DenistyMap found.");
  }

  if (ownPosition) {
    this->_currentCell = cellId;
  }
}

} /* namespace rover */
