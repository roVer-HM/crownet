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

template <typename NODE_ID>
class RegularGridMap
    : public PositionMap<
          std::pair<int, int>,
          CellEntry<DensityMeasure<NODE_ID>, DensityMeasureCtor<NODE_ID>>> {
 public:
  using CellId = std::pair<int, int>;
  using NodeId = NODE_ID;
  RegularGridMap(NodeId id, double gridSize);
  virtual ~RegularGridMap() = default;

  using PositionMap<CellId,
                    CellEntry<DensityMeasure<NODE_ID>,
                              DensityMeasureCtor<NODE_ID>>>::incrementLocal;
  virtual CellId incrementLocal(const inet::Coord& coord, const NodeId nodeId,
                                const omnetpp::simtime_t& t);

  virtual void incrementLocalOwnPos(const inet::Coord& coord,
                                    const omnetpp::simtime_t& t);

  double getGridSize() const;

 private:
  double gridSize;
};

/// implementation RegularGridMap<NODE_ID>
template <typename NODE_ID>
inline RegularGridMap<NODE_ID>::RegularGridMap(NodeId id, double gridSize)
    : PositionMap<CellId, CellEntry<DensityMeasure<NODE_ID>,
                                    DensityMeasureCtor<NODE_ID>>>(id),
      gridSize(gridSize) {}

template <typename NODE_ID>
inline typename RegularGridMap<NODE_ID>::CellId
RegularGridMap<NODE_ID>::incrementLocal(const inet::Coord& coord,
                                        const NodeId nodeId,
                                        const omnetpp::simtime_t& t) {
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

  return cellId;
}

template <typename NODE_ID>
inline void RegularGridMap<NODE_ID>::incrementLocalOwnPos(
    const inet::Coord& coord, const omnetpp::simtime_t& t) {
  this->_currentCell = this->incrementLocal(coord, this->getNodeId(), t);
}

template <typename NODE_ID>
inline double RegularGridMap<NODE_ID>::getGridSize() const {
  return this->gridSize;
}

} /* namespace rover */
