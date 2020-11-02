#pragma once

#include "rover/common/positionMap/PositionMap.h"

namespace rover {

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline void PositionMap<CELL_KEY, VALUE, CTOR>::initViews() {
  views["local"] = std::make_shared<LocalView>(this);
  views["ymf"] = std::make_shared<YmfView>(this);
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
typename PositionMap<CELL_KEY, VALUE, CTOR>::cell_mapped_type&
PositionMap<CELL_KEY, VALUE, CTOR>::getCellEntry(
    const cell_key_type& cell_key) {
  auto cellEntry = _map.find(cell_key);
  if (cellEntry != _map.end()) {
    return cellEntry->second;
  } else {
    // create new cell entry
    auto newCellEntry =
        _map.emplace(std::piecewise_construct, std::forward_as_tuple(cell_key),
                     std::forward_as_tuple(_cellCtor()));
    if (!newCellEntry.second) {
      throw omnetpp::cRuntimeError("error inserting cellEntry");
    } else {
      return newCellEntry.first->second;
    }
  }
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline typename PositionMap<CELL_KEY, VALUE, CTOR>::cellContainer_r
PositionMap<CELL_KEY, VALUE, CTOR>::range() {
  return boost::make_iterator_range(this->_map.begin(), this->_map.end());
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline void PositionMap<CELL_KEY, VALUE, CTOR>::clearMap(
    const node_time_type& t) {
  this->clearMap(_localNodeId, t);
  this->clearLocalNodetoCellMap();
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline void PositionMap<CELL_KEY, VALUE, CTOR>::clearMap(
    const node_key_type& node_key, const node_time_type& t) {
  for (auto& entry : this->_map) {
    entry.second.clear(node_key, t);
  }
  this->clearLocalNodetoCellMap();
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline void PositionMap<CELL_KEY, VALUE, CTOR>::resetMap(
    const node_time_type& t) {
  resetMap(_localNodeId, t);
  clearLocalNodetoCellMap();
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline void PositionMap<CELL_KEY, VALUE, CTOR>::resetMap(
    const node_key_type& node_key, const node_time_type& t) {
  for (auto& entry : _map) {
    entry.second.reset(node_key, t);
  }
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline void PositionMap<CELL_KEY, VALUE, CTOR>::update(
    const cell_key_type& cell_key, const node_key_type& node_key,
    node_mapped_type&& measure_value) {
  getCellEntry(cell_key).createOrUpdate(node_key, std::move(measure_value));
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline void PositionMap<CELL_KEY, VALUE, CTOR>::removeNeighbour(
    const node_key_type& neigbourId) {
  _localNodeToCellMap.erase(neigbourId);
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline bool PositionMap<CELL_KEY, VALUE, CTOR>::hasNeighbour(
    const node_key_type& neigbourId) const {
  return _localNodeToCellMap.find(neigbourId) != _localNodeToCellMap.end();
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline bool PositionMap<CELL_KEY, VALUE, CTOR>::hasNeighbour(
    const node_key_type& neigbourId, const cell_key_type& cellID) const {
  auto iter = _localNodeToCellMap.find(neigbourId);

  // neigbourId exists and  neighbour is in cellID
  return iter != _localNodeToCellMap.end() && iter->second == cellID;
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline const int PositionMap<CELL_KEY, VALUE, CTOR>::neighbourCount() const {
  return _localNodeToCellMap.size();
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline void PositionMap<CELL_KEY, VALUE, CTOR>::updateNeighbourCell(
    const node_key_type& neigbourId, const cell_key_type& cellId) {
  _localNodeToCellMap[neigbourId] = cellId;
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline void PositionMap<CELL_KEY, VALUE, CTOR>::clearLocalNodetoCellMap() {
  _localNodeToCellMap.clear();
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline std::shared_ptr<
    typename PositionMap<CELL_KEY, VALUE, CTOR>::PositionMapView>
PositionMap<CELL_KEY, VALUE, CTOR>::getView(std::string view_name) {
  auto iter = views.find(view_name);
  if (iter == views.end())
    omnetpp::cRuntimeError("View '%' not found ", view_name.c_str());

  return iter->second;
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline std::shared_ptr<
    typename PositionMap<CELL_KEY, VALUE, CTOR>::PositionMapView>
PositionMap<CELL_KEY, VALUE, CTOR>::getViewBySource(
    const typename PositionMap<CELL_KEY, VALUE, CTOR>::node_key_type& nodeId) {
  auto view =
      std::make_shared<PositionMap<CELL_KEY, VALUE, CTOR>::SourceNodeView>(
          this, nodeId);
  return std::move(view);
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline void PositionMap<CELL_KEY, VALUE, CTOR>::visit_entry(
    const PositionMap<CELL_KEY, VALUE, CTOR>::entry_visitor_f& visitor) const {
  for (const auto& cell : this->_map) {
    // for each cell...
    for (const auto& entry : cell.second.validRange()) {
      visitor(cell.first, entry);
    }
  }
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline void PositionMap<CELL_KEY, VALUE, CTOR>::clearEntrySelection() {
  using visitor_f =
      typename PositionMap<CELL_KEY, VALUE, CTOR>::entry_visitor_f;
  using cell_key_type =
      typename PositionMap<CELL_KEY, VALUE, CTOR>::cell_key_type;
  using node_value_type =
      typename PositionMap<CELL_KEY, VALUE, CTOR>::node_value_type;

  visitor_f f = [](const cell_key_type& cell, const node_value_type& entry) {
    entry.second->setSelectedIn("");
  };
  this->visit_entry(f);
}

}  // namespace rover
