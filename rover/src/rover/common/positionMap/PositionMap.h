
/*
 * PositionMap.h
 *
 *  Created on: Aug 25, 2020
 *      Author: sts
 */

#pragma once

#include <omnetpp/cexception.h>
#include <omnetpp/clog.h>
#include <boost/heap/binomial_heap.hpp>
#include <boost/range.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm/max_element.hpp>
#include <boost/range/algorithm/min_element.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <map>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include "rover/common/positionMap/Entry.h"

namespace rover {

template <typename VALUE, typename ENTRY_CTOR = EntryDefaultCtorImpl<VALUE>,
          typename std::enable_if<std::is_base_of<
              IEntry<typename VALUE::key_type, typename VALUE::time_type>,
              VALUE>::value>::type* = nullptr,
          typename std::enable_if<std::is_base_of<
              EntryCtor<VALUE>, ENTRY_CTOR>::value>::type* = nullptr>
class CellEntry {
 public:
  using key_type = typename VALUE::key_type;   // measuring node
  using mapped_type = std::shared_ptr<VALUE>;  // some type of IEntry
  using value_type = std::pair<const key_type&, mapped_type>;
  using entry_ctor = ENTRY_CTOR;

 private:
  using map_type = std::map<key_type, mapped_type>;
  map_type _data;
  key_type _localKey;
  std::shared_ptr<entry_ctor> _entryCtor;
  mapped_type _localEntry = nullptr;

 public:
  using map_unary_filter =
      std::function<bool(const typename map_type::value_type&)>;
  using map_binary_filter =
      std::function<bool(const typename map_type::value_type&,
                         const typename map_type::value_type&)>;
  using map_iter = typename map_type::iterator;
  using map_range = boost::iterator_range<typename map_type::iterator>;
  using map_range_filterd = boost::filtered_range<map_unary_filter, map_range>;

 public:
  template <typename T = ENTRY_CTOR>

  CellEntry(key_type localKey, std::shared_ptr<entry_ctor> ctor)
      : _localKey(localKey), _entryCtor(ctor) {}

  const bool hasLocalMeasure() const { return _localEntry != nullptr; }
  const bool hasValidLocalMeasure() const {
    return hasLocalMeasure() && getLocal()->valid();
  }

  const bool hasMeasure(const key_type& node_id) const {
    return _data.find(node_id) != _data.end();
  }

  const bool hasValidMeasure(const key_type& node_id) const {
    return hasMeasure(node_id) && get(node_id)->valid();
  }

  /**
   * reset local data in cell
   */
  void resetLocalMeasure() {
    if (hasLocalMeasure()) _localEntry->reset();
  }

  /**
   * reset all data in cell, local and received
   */
  void reset() {
    for (const auto& e : _data) {
      e.second->reset();
    }
  }

  void reset(const key_type& node_id) {
    if (hasMeasure(node_id)) {
      get(node_id)->reset();
    }
  }

  const key_type localKey() const { return _localKey; }

  //  const entry_ctor& getCtor() { return _entryCtor; }

  mapped_type getLocal() {
    if (!hasLocalMeasure()) {
      auto ret = _data.emplace(std::piecewise_construct,
                               std::forward_as_tuple(_localKey),
                               std::forward_as_tuple(_entryCtor->localEntry()));
      if (!ret.second) {
        throw omnetpp::cRuntimeError("element with key %s already existed.");
      }
      _localEntry = ret.first->second;  // address of mapped_type in map.
    }

    return _localEntry;
  }
  const mapped_type getLocal() const {
    return const_cast<CellEntry*>(this)->getLocal();
  }

  void createOrUpdate(const key_type& key, mapped_type&& val) {
    val->setSource(key);
    _data[key] = std::move(val);
  }

  mapped_type get(const key_type& key) {
    auto measure = _data.find(key);
    if (measure != _data.end()) {
      return measure->second;
    } else {
      auto newMeasure =
          _data.emplace(std::piecewise_construct, std::forward_as_tuple(key),
                        std::forward_as_tuple(_entryCtor->entry()));
      if (!newMeasure.second) {
        throw omnetpp::cRuntimeError("error inserting newMeasure");
      } else {
        return newMeasure.first->second;
      }
    }
  }
  const mapped_type get(const key_type& key) const {
    return const_cast<CellEntry*>(this)->get(key);
  }

  mapped_type youngestMeasureFirst(bool prefereLocal = true) {
    map_binary_filter _f = [](const typename map_type::value_type lhs,
                              const typename map_type::value_type rhs) -> bool {
      // Comparator true if lhs lessThan rhs
      return lhs.second->compareMeasureTime(*rhs.second.get()) < 0;
    };
    auto range = validRange();
    //    auto ret = boost::range::min_element(rangge, _f);
    auto ret = boost::range::max_element(range, _f);

    if (prefereLocal && hasValidLocalMeasure() &&
        ret->second->compareMeasureTime(*_localEntry.get()) == 0) {
      // return local measure if it has the same age.
      return _localEntry;
    } else {
      return ret->second;
    }
  }
  const mapped_type youngestMeasureFirst() const {
    return const_cast<CellEntry*>(this)->youngestMeasureFirst();
  }

  const bool hasValid() const { return boost::size(validRange()); }

  map_range_filterd validRange() {
    map_unary_filter _f = [](const typename map_type::value_type val) -> bool {
      return val.second->valid();
    };
    using namespace boost::adaptors;
    map_range rng = boost::make_iterator_range(_data.begin(), _data.end());
    return rng | filtered(_f);
  }
  const map_range_filterd validRange() const {
    return const_cast<CellEntry*>(this)->validRange();
  }

  const int size() const { return boost::size(validRange()); }

  std::string str() const {
    std::ostringstream s;
    for (const auto entry : _data) {
      s << entry.second.str() << "\n";
    }
    return s.str();
  }
};

template <typename VALUE, typename ENTRY_KEY>
class CellCtor {
 public:
  CellCtor(ENTRY_KEY key, std::shared_ptr<typename VALUE::entry_ctor> eCtor)
      : key(key), entryCtor(eCtor) {}
  VALUE operator()() { return VALUE(key, entryCtor); }
  std::shared_ptr<typename VALUE::entry_ctor> getEntryCtor() const {
    return entryCtor;
  }

 private:
  ENTRY_KEY key;
  std::shared_ptr<typename VALUE::entry_ctor> entryCtor;
};

template <typename CELL_KEY, typename VALUE,
          typename CTOR = CellCtor<VALUE, typename VALUE::key_type>>
class PositionMap {
 public:
  class PositionMapView;
  class LocalView;
  class YmfView;

 public:
  using cell_key_type = CELL_KEY;  // key for one cell or triangle (bucket)
  using cell_mapped_type = VALUE;  // container in each bucket.
  using cell_value_type = std::pair<const cell_key_type&, cell_mapped_type&>;
  using cell_ctor_type = CTOR;  // creator for container

  // each bucket contains an entry map

  using node_key_type = typename VALUE::key_type;
  using node_mapped_type = typename VALUE::mapped_type;
  using node_value_type = typename VALUE::value_type;
  using node_ctor_type = typename VALUE::entry_ctor;

  // map one entry_mapped_type instance  to one key_type instance
  // used by boost::iterator_ranges to filter/aggregate the correct
  // entry_mapped_type based on given predicate/transformer.
  using view_value_type = std::pair<const cell_key_type&, node_mapped_type>;

 protected:
  using map_type = std::map<cell_key_type, cell_mapped_type>;
  using map_views = std::map<std::string, std::shared_ptr<PositionMapView>>;
  map_type _map;  // map of cells. cell -> node -> measure
  std::map<node_key_type, cell_key_type>
      _localNodeToCellMap;  // node -> cell (local map)
  map_views views;

  // identifier that maps to local entry_mapped_type value
  node_key_type _localNodeId;  // NodeId of this node.
  cell_key_type _currentCell;  // CellId this node  currently ocupies.
  cell_ctor_type _cellCtor;

 public:
  // functions
  using cellContainerFilter_f =
      std::function<bool(const typename map_type::value_type&)>;

  using cellEntryFilter_f = std::function<bool(const node_mapped_type&)>;
  using cellEntryTransform_f =
      std::function<view_value_type(typename map_type::value_type&)>;

  // ranges
  using cellContainer_r = boost::iterator_range<typename map_type::iterator>;
  using cellContainerFiltered_r =
      const boost::filtered_range<cellContainerFilter_f, cellContainer_r>;
  using cellEntryFiltered_r = boost::transformed_range<
      cellEntryTransform_f,
      const boost::filtered_range<cellContainerFilter_f, cellContainer_r>>;
  using cellEntryFiltered_iter = typename cellEntryFiltered_r::iterator;
  using cellEntryFiltered_const_iter =
      typename cellEntryFiltered_r::const_iterator;

 public:
  virtual ~PositionMap() = default;

  PositionMap(node_key_type localId)
      : _localNodeId(localId),
        _cellCtor(localId, std::make_shared<node_ctor_type>()) {
    initViews();
  }

 private:
  void initViews() {
    views["local"] = std::make_shared<LocalView>(this);
    views["ymf"] = std::make_shared<YmfView>(this);
  }

 public:
  cell_mapped_type& getCellEntry(const cell_key_type& cell_key) {
    auto cellEntry = _map.find(cell_key);
    if (cellEntry != _map.end()) {
      return cellEntry->second;
    } else {
      // create new cell entry
      auto newCellEntry = _map.emplace(std::piecewise_construct,
                                       std::forward_as_tuple(cell_key),
                                       std::forward_as_tuple(_cellCtor()));
      if (!newCellEntry.second) {
        throw omnetpp::cRuntimeError("error inserting cellEntry");
      } else {
        return newCellEntry.first->second;
      }
    }
  }

  cellContainer_r range() {
    return boost::make_iterator_range(_map.begin(), _map.end());
  }

  void resetMap() {
    resetMap(_localNodeId);
    clearLocalNodetoCellMap();
  }

  void resetMap(const node_key_type& node_key) {
    for (auto& entry : _map) {
      entry.second.reset(node_key);
    }
  }

  /**
   * :cell_key:       cellId to which the measurement belongs
   * :node_key:       nodeId of node which provided the measurement. Does not
   *                  have to be the original creator of measurement.
   * :measure_value:  timestamped
   *                  density count for cellId, provided by nodeId.
   */
  void update(const cell_key_type& cell_key, const node_key_type& node_key,
              node_mapped_type&& measure_value) {
    getCellEntry(cell_key).createOrUpdate(node_key, std::move(measure_value));
  }

  void removeNeighbour(const node_key_type& neigbourId) {
    _localNodeToCellMap.erase(neigbourId);
  }

  bool hasNeighbour(const node_key_type& neigbourId) const {
    return _localNodeToCellMap.find(neigbourId) != _localNodeToCellMap.end();
  }

  bool hasNeighbour(const node_key_type& neigbourId,
                    const cell_key_type& cellID) const {
    auto iter = _localNodeToCellMap.find(neigbourId);

    // neigbourId exists and  neighbour is in cellID
    return iter != _localNodeToCellMap.end() && iter->second == cellID;
  }

  const int neighbourCount() const { return _localNodeToCellMap.size(); }

  void updateNeighbourCell(const node_key_type& neigbourId,
                           const cell_key_type& cellId) {
    _localNodeToCellMap[neigbourId] = cellId;
  }

  void clearLocalNodetoCellMap() { _localNodeToCellMap.clear(); }

  std::shared_ptr<PositionMapView> getView(std::string view_name) {
    auto iter = views.find(view_name);
    if (iter == views.end())
      omnetpp::cRuntimeError("View '%' not found ", view_name.c_str());

    return iter->second;
  }

  std::shared_ptr<PositionMapView> getViewBySource(
      const node_key_type& nodeId) {
    auto view = std::make_shared<SourceNodeView>(this, nodeId);
    return std::move(view);
  }

  node_key_type getNodeId() const { return _localNodeId; }

  cell_key_type getCellId() const { return _currentCell; }

  class PositionMapView {
   public:
    PositionMapView() : _cell_map(nullptr), _view_name("") {}
    PositionMapView(
        PositionMap<cell_key_type, cell_mapped_type, cell_ctor_type>* map,
        std::string view_name)
        : _cell_map(map), _view_name(view_name) {}

   public:
    virtual ~PositionMapView() = default;
    virtual cellEntryFiltered_r range() = 0;
    const cellEntryFiltered_r range() const;
    cellEntryFiltered_iter begin();
    cellEntryFiltered_iter end();
    cellEntryFiltered_const_iter end() const;

    std::string str() const;
    int size() const;
    node_key_type getId() const;

    cellEntryFiltered_iter find(const cell_key_type& k);
    cellEntryFiltered_iter find(const cell_key_type& k) const;
    const bool hasValue(const cell_key_type& k) const;

   protected:
    PositionMap<cell_key_type, cell_mapped_type, cell_ctor_type>* _cell_map;
    std::string _view_name;
  };

  class LocalView : public PositionMapView {
   public:
    LocalView(PositionMap<cell_key_type, cell_mapped_type, cell_ctor_type>* map)
        : PositionMapView(map, "local") {}

    virtual cellEntryFiltered_r range() override {
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
  };

  class YmfView : public PositionMapView {
   public:
    YmfView(PositionMap<cell_key_type, cell_mapped_type, cell_ctor_type>* map)
        : PositionMapView(map, "ymf") {}

    virtual cellEntryFiltered_r range() override {
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
  };

  class SourceNodeView : public PositionMapView {
   public:
    SourceNodeView(
        PositionMap<cell_key_type, cell_mapped_type, cell_ctor_type>* map,
        node_key_type sourceNodeId)
        : PositionMapView(map, ""), sourceNodeId(sourceNodeId) {}

    virtual cellEntryFiltered_r range() override {
      // filter: only valid measurements
      cellContainerFilter_f _f =
          [this](const typename map_type::value_type& map_value) {
            auto& value = map_value.second;
            return value.hasValidMeasure(this->sourceNodeId);
          };

      // transform
      cellEntryTransform_f _t =
          [this](typename map_type::value_type& map_value) {
            view_value_type ret = view_value_type{
                map_value.first, map_value.second.get(this->sourceNodeId)};
            return ret;
          };

      using namespace boost::adaptors;
      cellContainer_r range_all = this->_cell_map->range();

      return range_all | filtered(_f) | transformed(_t);
    }

   protected:
    node_key_type sourceNodeId;
  };
};

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

} /* namespace rover */
