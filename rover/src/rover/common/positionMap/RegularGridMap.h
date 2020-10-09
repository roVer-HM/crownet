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
#include "traci/Position.h"

namespace rover {

template <typename NODE_ID>
class RegularGridMap
    : public PositionMap<
          std::pair<int, int>,
          CellEntry<DensityMeasure<NODE_ID>, DensityMeasureCtor<NODE_ID>>> {
 public:
  class FullIter;
  using CellId = std::pair<int, int>;
  using NodeId = NODE_ID;
  RegularGridMap(NodeId id, double gridSize, std::pair<int, int> gridDim);
  virtual ~RegularGridMap() = default;

  virtual typename RegularGridMap<NODE_ID>::CellId incrementLocal(
      const traci::TraCIPosition& coord, const NodeId nodeId,
      const omnetpp::simtime_t& t);
  virtual typename RegularGridMap<NODE_ID>::CellId incrementLocal(
      const CellId& cellId, const NodeId nodeId, const omnetpp::simtime_t& t);

  virtual void incrementLocalOwnPos(const traci::TraCIPosition& coord,
                                    const omnetpp::simtime_t& t);

  virtual void moveNodeInLocalMap(const CellId& cellId, const NodeId& nodeId);

  double getGridSize() const;
  const std::pair<int, int> getGridDim() const;
  FullIter iter(const std::string view, bool rowMajorOrder = true);

 private:
  double gridSize;
  std::pair<int, int> gridDim;

 public:
  using iter_value = typename RegularGridMap<NODE_ID>::node_mapped_type;
  using view_type = typename RegularGridMap<NODE_ID>::PositionMapView;
  using range_type = typename RegularGridMap<NODE_ID>::cellEntryFiltered_r;
  /**
   * Iterator for regular grid in row major or column major order based on
   * given view.
   *
   */
  class FullIter : public std::iterator<std::output_iterator_tag, iter_value> {
   public:
    explicit FullIter(RegularGridMap<NODE_ID>& grid, std::string view_str,
                      bool rowMajorOrder = true);
    iter_value operator*() const;
    FullIter& operator++();
    FullIter operator++(int);
    bool operator==(const FullIter& rhs);
    bool operator!=(const FullIter& rhs) { return !operator==(rhs); }

    FullIter begin();
    FullIter end();

    const int getMaxIndex() const;
    const int getIndex() const;
    const std::pair<int, int> getKey() const;

   private:
    explicit FullIter(FullIter& other, int index);
    std::shared_ptr<view_type> view;
    std::pair<int, int> gridDim;
    std::shared_ptr<typename RegularGridMap<NODE_ID>::node_ctor_type> ctor;
    bool rowMajorOrder;
    int index;
  };
};

template <typename NODE_ID>
inline RegularGridMap<NODE_ID>::FullIter::FullIter(
    RegularGridMap<NODE_ID>& grid, std::string view_str, bool rowMajorOrder)
    : view(grid.getView(view_str)),
      gridDim(grid.getGridDim()),
      ctor(grid._cellCtor.getEntryCtor()),
      rowMajorOrder(rowMajorOrder),
      index(0) {}

template <typename NODE_ID>
inline RegularGridMap<NODE_ID>::FullIter::FullIter(FullIter& other, int index)
    : view(other.view),
      gridDim(other.gridDim),
      ctor(other.ctor),
      rowMajorOrder(other.rowMajorOrder),
      index(other.index) {}

template <typename NODE_ID>
inline typename RegularGridMap<NODE_ID>::FullIter&
RegularGridMap<NODE_ID>::FullIter::operator++() {
  index++;
  if (index > getMaxIndex()) {
    throw std::out_of_range("Out of Range Exception!");
  }
  return *this;
}

template <typename NODE_ID>
inline typename RegularGridMap<NODE_ID>::FullIter
RegularGridMap<NODE_ID>::FullIter::operator++(int) {
  typename RegularGridMap<NODE_ID>::FullIter tmp(*this);
  this->operator++();
  return tmp;
}

template <typename NODE_ID>
inline bool RegularGridMap<NODE_ID>::FullIter::operator==(const FullIter& rhs) {
  return index == rhs.index && view == rhs.view &&
         rowMajorOrder == rhs.rowMajorOrder && gridDim == rhs.gridDim;
}

template <typename NODE_ID>
inline typename RegularGridMap<NODE_ID>::FullIter
RegularGridMap<NODE_ID>::FullIter::begin() {
  return RegularGridMap<NODE_ID>::FullIter(*this, 0);
}

template <typename NODE_ID>
inline typename RegularGridMap<NODE_ID>::FullIter
RegularGridMap<NODE_ID>::FullIter::end() {
  return RegularGridMap<NODE_ID>::FullIter(*this, this->getMaxIndex());
}

template <typename NODE_ID>
inline const int RegularGridMap<NODE_ID>::FullIter::getMaxIndex() const {
  return this->gridDim.first * this->gridDim.second;
}

template <typename NODE_ID>
inline const int RegularGridMap<NODE_ID>::FullIter::getIndex() const {
  return this->index;
}

template <typename NODE_ID>
inline const std::pair<int, int> RegularGridMap<NODE_ID>::FullIter::getKey()
    const {
  std::pair<int, int> key;
  if (this->rowMajorOrder) {
    key.first = this->index % this->gridDim.first;
    key.second = floor(this->index / this->gridDim.first);
  } else {
    key.first = floor(this->index / this->gridDim.second);
    key.second = this->index % this->gridDim.second;
  }
  return key;
}

template <typename NODE_ID>
inline typename RegularGridMap<NODE_ID>::iter_value
    RegularGridMap<NODE_ID>::FullIter::operator*() const {
  // 1 use index, gridDim and rowMajorOrder  to generate the current grid index
  std::pair<int, int> key = this->getKey();

  // 2 check the underlying range if value exist
  auto iter = this->view->find(key);
  if (iter != this->view->end()) {
    // 2a if present return found value
    return iter->second;
  } else {
    // 2b if missing return empty value given on ctor
    // return empty
    return ctor->empty();
  }
}

/// implementation RegularGridMap<NODE_ID>
template <typename NODE_ID>
inline RegularGridMap<NODE_ID>::RegularGridMap(NodeId id, double gridSize,
                                               std::pair<int, int> gridDim)
    : PositionMap<CellId, CellEntry<DensityMeasure<NODE_ID>,
                                    DensityMeasureCtor<NODE_ID>>>(id),
      gridSize(gridSize),
      gridDim(gridDim) {}

template <typename NODE_ID>
inline typename RegularGridMap<NODE_ID>::CellId
RegularGridMap<NODE_ID>::incrementLocal(const traci::TraCIPosition& coord,
                                        const NodeId nodeId,
                                        const omnetpp::simtime_t& t) {
  CellId cellId =
      std::make_pair(floor(coord.x / gridSize), floor(coord.y / gridSize));
  return this->incrementLocal(cellId, nodeId, t);
}

/**
 * Increment local count in cellId.
 * The node with :nodeId: is added to the count. Check that this node is added
 * only once.
 */
template <typename NODE_ID>
inline typename RegularGridMap<NODE_ID>::CellId
RegularGridMap<NODE_ID>::incrementLocal(const CellId& cellId,
                                        const NodeId nodeId,
                                        const omnetpp::simtime_t& t) {
  // 0) each node must be added only once to the local map
  if (this->hasNeighbour(nodeId)) {
    throw omnetpp::cRuntimeError("duplicate node in DenistyMap found.");
  }

  // 1) increment local count in cell
  std::shared_ptr<LocalDensityMeasure<NODE_ID>> locMeasure =
      std::static_pointer_cast<LocalDensityMeasure<NODE_ID>>(
          this->getCellEntry(cellId).getLocal());
  locMeasure->incrementCount(t);

  // 2) add nodeId into nodeId set
  locMeasure->nodeIds.insert(nodeId);

  // 3) add mapping nodeId->cellId into updateNeighbourCell
  this->updateNeighbourCell(nodeId, cellId);
  return cellId;
}

template <typename NODE_ID>
inline void RegularGridMap<NODE_ID>::incrementLocalOwnPos(
    const traci::TraCIPosition& coord, const omnetpp::simtime_t& t) {
  this->_currentCell = this->incrementLocal(coord, this->getNodeId(), t);
}

/**
 * Move :nodeId: to cell given by :cellId:.
 * - If :nodeId: already exist in some other
 *   cell remove it form there.
 * - If :nodeId: already exists in :cellId: do nothing.
 * - If :nodeId: does not exist anywhere add it to :cellId: in the local map.
 */
template <typename NODE_ID>
inline void RegularGridMap<NODE_ID>::moveNodeInLocalMap(const CellId& cellId,
                                                        const NodeId& nodeId) {
  auto iter = this->_localNodeToCellMap.find(nodeId);
  if (iter == this->_localNodeToCellMap.end()) {
    // nodeId does not exist. add :cellId: to local map
    this->incrementLocal(cellId, nodeId, omnetpp::SimTime());

  } else if (iter->second != cellId) {
    // nodeId present but in wrong cell. Move :nodeId: to :cellId:

    // remove from old cell
    auto oldMeasure = std::static_pointer_cast<LocalDensityMeasure<NODE_ID>>(
        this->getCellEntry(iter->second).getLocal());
    oldMeasure->decrementCount(omnetpp::SimTime());
    oldMeasure->nodeIds.erase(nodeId);
    this->removeNeighbour(nodeId);

    // add to new cell
    this->incrementLocal(cellId, nodeId, omnetpp::SimTime());
  } else {
    // :nodeId: is already in :cellId:. Do nothing.
  }
}

template <typename NODE_ID>
inline double RegularGridMap<NODE_ID>::getGridSize() const {
  return this->gridSize;
}

template <typename NODE_ID>
inline const std::pair<int, int> RegularGridMap<NODE_ID>::getGridDim() const {
  return this->gridDim;
}

template <typename NODE_ID>
inline typename RegularGridMap<NODE_ID>::FullIter RegularGridMap<NODE_ID>::iter(
    const std::string view, bool rowMajorOrder) {
  return RegularGridMap<NODE_ID>::FullIter(*this, view, rowMajorOrder);
}

} /* namespace rover */
