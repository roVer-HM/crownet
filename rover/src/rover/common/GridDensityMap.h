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
#include "rover/common/PositionMap.h"

namespace rover {

class DensityMeasure : public IEntry<omnetpp::simtime_t> {
 public:
  DensityMeasure();
  DensityMeasure(int, omnetpp::simtime_t&, omnetpp::simtime_t&);

  void reset() override;
  const bool valid() const override;
  void incrementCount(const omnetpp::simtime_t& t) override;

  friend std::ostream& operator<<(std::ostream& os, const DensityMeasure& obj);

 private:
  int count;
  omnetpp::simtime_t measurement_time;
  omnetpp::simtime_t received_time;
  bool _valid;
};

template <typename NodeID>
class GridDensityMap {
 public:
  using cellId = std::pair<int, int>;
  using nodeId = NodeID;
  using map_type = PositionMap<cellId, CellEntry<nodeId, DensityMeasure>>;

  virtual ~GridDensityMap() = default;
  GridDensityMap(nodeId id, double gridSize)
      : _nodeId(id), _map(id), gridSize(gridSize) {}

  void resetLocalMap() { _map.resetLocalMap(); }
  void updateLocalMap(const cellId& _cellId, DensityMeasure& measure) {
    _map.updateLocal(_cellId, measure);
  }

  void incrementLocal(const inet::Coord& coord, const omnetpp::simtime_t& t,
                      bool ownPosition = false) {
    cellId id =
        std::make_pair(floor(coord.x / gridSize), floor(coord.y / gridSize));
    _map.incrementLocal(id, t);
    if (ownPosition) nodeCurrentCell = id;
  }
  void printLocalMap() {
    using namespace omnetpp;
    EV_DEBUG << "GridDensityMap (NodeId: " << _nodeId << " Cell("
             << nodeCurrentCell.first << ", " << nodeCurrentCell.second
             << ")\n";
    _map.printLocalMap();
  }

  const std::string& getId() const { return _nodeId; }

  std::string _nodeId;

 private:
  map_type _map;
  double gridSize;
  cellId nodeCurrentCell;
};

} /* namespace rover */
