#pragma once

#include "rover/common/positionMap/PositionMap.h"

namespace rover {

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline int PositionMap<CELL_KEY, VALUE, CTOR>::PositionMapView::size() const {
  return boost::size(this->range());
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline std::string PositionMap<CELL_KEY, VALUE, CTOR>::PositionMapView::str()
    const {
  std::stringstream s;
  s << "Map[ " << _view_name << "] (NodeId: " << this->_cell_map->_localNodeId
    << "\n";
  for (auto entry : this->range()) {
    s << "   Cell(" << entry.first.first << ", " << entry.first.second << ") "
      << entry.second->str() << std::endl;
  }
  return s.str();
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline const typename PositionMap<CELL_KEY, VALUE, CTOR>::cellEntryFiltered_r
PositionMap<CELL_KEY, VALUE, CTOR>::PositionMapView::range() const {
  return const_cast<PositionMapView*>(this)->range();
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline typename PositionMap<CELL_KEY, VALUE, CTOR>::cellEntryFiltered_iter
PositionMap<CELL_KEY, VALUE, CTOR>::PositionMapView::begin() {
  return this->range().begin();
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline typename PositionMap<CELL_KEY, VALUE, CTOR>::cellEntryFiltered_iter
PositionMap<CELL_KEY, VALUE, CTOR>::PositionMapView::end() {
  return this->range().end();
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline typename PositionMap<CELL_KEY, VALUE, CTOR>::cellEntryFiltered_const_iter
PositionMap<CELL_KEY, VALUE, CTOR>::PositionMapView::end() const {
  return this->range().end();
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline typename PositionMap<CELL_KEY, VALUE, CTOR>::node_key_type
PositionMap<CELL_KEY, VALUE, CTOR>::PositionMapView::getId() const {
  return this->_cell_map->_localNodeId;
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline typename PositionMap<CELL_KEY, VALUE, CTOR>::cellEntryFiltered_iter
PositionMap<CELL_KEY, VALUE, CTOR>::PositionMapView::find(
    const cell_key_type& k) {
  auto rng = this->range();
  auto it = boost::range::find_if(
      rng, [&k](std::pair<const cell_key_type&, node_mapped_type> _item) {
        return k == _item.first;
      });
  return it;
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline typename PositionMap<CELL_KEY, VALUE, CTOR>::cellEntryFiltered_iter
PositionMap<CELL_KEY, VALUE, CTOR>::PositionMapView::find(
    const cell_key_type& k) const {
  return const_cast<PositionMapView*>(this)->find(k);
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline const bool PositionMap<CELL_KEY, VALUE, CTOR>::PositionMapView::hasValue(
    const cell_key_type& k) const {
  auto iter = this->find(k);
  return iter != this->end();
}

template <typename CELL_KEY, typename VALUE, typename CTOR>
using cellContainer_r =
    typename PositionMap<CELL_KEY, VALUE, CTOR>::cellContainer_r;

template <typename CELL_KEY, typename VALUE, typename CTOR>
using cellContainerFiltered_r =
    typename PositionMap<CELL_KEY, VALUE, CTOR>::cellContainerFiltered_r;

template <typename CELL_KEY, typename VALUE, typename CTOR>
using cellEntryFiltered_r =
    typename PositionMap<CELL_KEY, VALUE, CTOR>::cellEntryFiltered_r;

template <typename CELL_KEY, typename VALUE, typename CTOR>
using cellEntryFiltered_iter =
    typename PositionMap<CELL_KEY, VALUE, CTOR>::cellEntryFiltered_iter;

template <typename CELL_KEY, typename VALUE, typename CTOR>
using cellEntryFiltered_const_iter =
    typename PositionMap<CELL_KEY, VALUE, CTOR>::cellEntryFiltered_const_iter;

template <typename CELL_KEY, typename VALUE, typename CTOR>
using cellContainerFilter_f =
    typename PositionMap<CELL_KEY, VALUE, CTOR>::cellContainerFilter_f;

template <typename CELL_KEY, typename VALUE, typename CTOR>
using cellEntryFilter_f =
    typename PositionMap<CELL_KEY, VALUE, CTOR>::cellEntryFilter_f;

template <typename CELL_KEY, typename VALUE, typename CTOR>
using cellEntryTransform_f =
    typename PositionMap<CELL_KEY, VALUE, CTOR>::cellEntryTransform_f;

/// local

template <typename CELL_KEY, typename VALUE, typename CTOR>
inline cellEntryFiltered_r<CELL_KEY, VALUE, CTOR>
PositionMap<CELL_KEY, VALUE, CTOR>::LocalView::range() {
  // filter: only valid and local measure
  cellContainerFilter_f _f =
      [](const typename map_type::value_type& map_value) {
        auto& value = map_value.second;
        return value.hasValidLocalMeasure();
      };

  // transform
  cellEntryTransform_f _t = [](typename map_type::value_type& map_value) {
    view_value_type ret =
        view_value_type{map_value.first, map_value.second.getLocal()};
    return ret;
  };

  using namespace boost::adaptors;
  cellContainer_r range_all = this->_cell_map->range();

  return range_all | filtered(_f) | transformed(_t);
}

/// ymf
template <typename CELL_KEY, typename VALUE, typename CTOR>
inline cellEntryFiltered_r<CELL_KEY, VALUE, CTOR>
PositionMap<CELL_KEY, VALUE, CTOR>::YmfView::range() {
  // filter: only valid measurements
  cellContainerFilter_f _f =
      [](const typename map_type::value_type& map_value) {
        auto& value = map_value.second;
        return value.hasValid();
      };

  // transform
  cellEntryTransform_f _t = [](typename map_type::value_type& map_value) {
    view_value_type ret = view_value_type{
        map_value.first, map_value.second.youngestMeasureFirst()};
    return ret;
  };

  using namespace boost::adaptors;
  cellContainer_r range_all = this->_cell_map->range();

  return range_all | filtered(_f) | transformed(_t);
}

/// SourceNodeView
template <typename CELL_KEY, typename VALUE, typename CTOR>
inline cellEntryFiltered_r<CELL_KEY, VALUE, CTOR>
PositionMap<CELL_KEY, VALUE, CTOR>::SourceNodeView::range() {
  // filter: only valid measurements
  cellContainerFilter_f _f =
      [this](const typename map_type::value_type& map_value) {
        auto& value = map_value.second;
        return value.hasValidMeasure(this->sourceNodeId);
      };

  // transform
  cellEntryTransform_f _t = [this](typename map_type::value_type& map_value) {
    view_value_type ret = view_value_type{
        map_value.first, map_value.second.get(this->sourceNodeId)};
    return ret;
  };

  using namespace boost::adaptors;
  cellContainer_r range_all = this->_cell_map->range();

  return range_all | filtered(_f) | transformed(_t);
}

}  // namespace rover
