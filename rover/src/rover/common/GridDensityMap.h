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
  DensityMeasure(int, omnetpp::simtime_t& measurement_time,
                 omnetpp::simtime_t& received_time);

  virtual std::string delimWith(std::string delimiter) const override;

  friend std::ostream& operator<<(std::ostream& os, const DensityMeasure& obj);
};

template <typename NodeID>
class GridDensityMap {
 public:
  using CellId = std::pair<int, int>;
  using NodeId = NodeID;
  using map_type = PositionMap<CellId, CellEntry<NodeId, DensityMeasure>>;

  using PositionMapView = typename map_type::PositionMapView;
  using view_visitor =
      std::function<void(const CellId&, const DensityMeasure&)>;

 private:
  map_type _map;
  double gridSize;

 public:
  virtual ~GridDensityMap() = default;
  GridDensityMap(NodeId id, double gridSize) : _map(id), gridSize(gridSize) {}

  void resetLocalMap() { _map.resetLocalMap(); }

  void incrementLocal(const inet::Coord& coord, const omnetpp::simtime_t& t,
                      bool ownPosition = false) {
    CellId id =
        std::make_pair(floor(coord.x / gridSize), floor(coord.y / gridSize));
    _map.incrementLocal(id, t, ownPosition);
  }

  void updateMap(const CellId& _cellId, const NodeId& _nodeId,
                 DensityMeasure& measure) {
    _map.update(_cellId, _nodeId, measure);
  }

  std::shared_ptr<PositionMapView> getView(std::string view_name) {
    return _map.getView(view_name);
  }
};

} /* namespace rover */
