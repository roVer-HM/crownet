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

  friend std::ostream& operator<<(std::ostream& os, const DensityMeasure& obj);
};

template <typename NodeID>
class GridDensityMap {
 public:
  using cellId = std::pair<int, int>;
  using nodeId = NodeID;
  using map_type = PositionMap<cellId, CellEntry<nodeId, DensityMeasure>>;

  using MapView = typename map_type::View;
  using view_visitor =
      std::function<void(const cellId&, const DensityMeasure&)>;

 private:
  std::string _nodeId;
  map_type _map;
  double gridSize;
  cellId nodeCurrentCell;

 public:
  virtual ~GridDensityMap() = default;
  GridDensityMap(nodeId id, double gridSize)
      : _nodeId(id), _map(id), gridSize(gridSize) {}

  void resetLocalMap() { _map.resetLocalMap(); }

  void incrementLocal(const inet::Coord& coord, const omnetpp::simtime_t& t,
                      bool ownPosition = false) {
    cellId id =
        std::make_pair(floor(coord.x / gridSize), floor(coord.y / gridSize));
    _map.incrementLocal(id, t);
    if (ownPosition) nodeCurrentCell = id;
  }

  void updateMap(const cellId& _cellId, const nodeId& _nodeId,
                 DensityMeasure& measure) {
    _map.update(_cellId, _nodeId, measure);
  }

  std::string strView(MapView view = MapView::LOCAL) {
    std::stringstream s;
    s << "GridDensityMap[ " << view << "] (NodeId: " << _nodeId << " Cell("
      << nodeCurrentCell.first << ", " << nodeCurrentCell.second << ")\n";
    s << _map.strView(view);
    return s.str();
  }

  void printView(MapView view = MapView::LOCAL) {
    using namespace omnetpp;
    EV_DEBUG << strView(view);
  }

  void visit(const view_visitor v, const MapView& view) const {
    for (const auto& entry : _map.getView(view)) {
      v(entry.first, entry.second);
    }
  }
  const int size(MapView view = MapView::LOCAL) const {
    return _map.size(view);
  }

  const std::string& getId() const { return _nodeId; }
};

} /* namespace rover */
