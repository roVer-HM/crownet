
/*
 * PositionMap.h
 *
 *  Created on: Aug 25, 2020
 *      Author: sts
 */

#pragma once

#include <omnetpp/cexception.h>
#include <omnetpp/clog.h>
#include <omnetpp/cobject.h>
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

namespace foobar {}

namespace rover {

template <typename VALUE,
          typename ENTRY_CTOR = EntryDefaultCtorImpl<typename VALUE::key_type,
                                                     typename VALUE::time_type>,
          typename std::enable_if<std::is_base_of<
              IEntry<typename VALUE::key_type, typename VALUE::time_type>,
              VALUE>::value>::type* = nullptr,
          typename std::enable_if<std::is_base_of<
              EntryCtor<typename VALUE::key_type, typename VALUE::time_type>,
              ENTRY_CTOR>::value>::type* = nullptr>
class CellEntry;

template <typename VALUE, typename ENTRY_CTOR>
inline std::ostream& operator<<(std::ostream& os,
                                CellEntry<VALUE, ENTRY_CTOR>& e) {
  return os << "Foo";
}

template <typename VALUE, typename ENTRY_CTOR,
          typename std::enable_if<std::is_base_of<
              IEntry<typename VALUE::key_type, typename VALUE::time_type>,
              VALUE>::value>::type*,
          typename std::enable_if<std::is_base_of<
              EntryCtor<typename VALUE::key_type, typename VALUE::time_type>,
              ENTRY_CTOR>::value>::type*>
class CellEntry {
 public:
  using key_type = typename VALUE::key_type;  // measuring node
  using time_type = typename VALUE::time_type;
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

  CellEntry(key_type localKey)
      : CellEntry(localKey, std::make_shared<entry_ctor>()) {}

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

  friend std::ostream& operator<<(std::ostream& os,
                                  const CellEntry<VALUE, ENTRY_CTOR>& e);

  /**
   * reset all data in cell, local and received
   */
  void resetAll(const time_type& t) {
    for (const auto& e : _data) {
      e.second->reset(t);
    }
  }

  /**
   * reset local data in cell
   */
  void reset(const time_type& t) {
    if (hasLocalMeasure()) _localEntry->reset(t);
  }

  void reset(const key_type& node_id, const time_type& t) {
    if (hasMeasure(node_id)) {
      get(node_id)->reset(t);
    }
  }

  void clearAll(const time_type& t) {
    for (const auto& e : _data) {
      e.second->clear(t);
    }
  }

  void clear(const time_type& t) {
    if (hasLocalMeasure()) _localEntry->clear(t);
  }

  void clear(const key_type& node_id, const time_type& t) {
    if (hasMeasure(node_id)) {
      get(node_id)->clear(t);
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
      _localEntry->reset();
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
class PositionMap : public omnetpp::cObject {
 public:
  class PositionMapView;
  class LocalView;
  class YmfView;
  class AllView;

 public:
  using cell_key_type = CELL_KEY;  // key for one cell or triangle (bucket)
  using cell_mapped_type = VALUE;  // container in each bucket.
  using cell_value_type = std::pair<const cell_key_type&, cell_mapped_type&>;
  using cell_ctor_type = CTOR;  // creator for container

  // each bucket contains an entry map

  using node_key_type = typename VALUE::key_type;
  using node_time_type = typename VALUE::time_type;
  using node_mapped_type = typename VALUE::mapped_type;
  using node_value_type = typename VALUE::value_type;
  using node_ctor_type = typename VALUE::entry_ctor;

  // map one entry_mapped_type instance  to one key_type instance
  // used by boost::iterator_ranges to filter/aggregate the correct
  // entry_mapped_type based on given predicate/transformer.
  using view_value_type = std::pair<const cell_key_type&, node_mapped_type>;
  using map_type = std::map<cell_key_type, cell_mapped_type>;

 protected:
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

  // entry_visitor (CellId, [NodeId, Entry])
  using entry_visitor_f =
      std::function<void(const cell_key_type&, const node_value_type&)>;

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
  void initViews();

 public:
  map_type getMap() const { return this->_map; }

  cell_mapped_type& getCellEntry(const cell_key_type& cell_key);

  cellContainer_r range();
  void clearMap(const node_time_type& t);
  void clearMap(const node_key_type& node_key, const node_time_type& t);
  void resetMap(const node_time_type& t);
  void resetMap(const node_key_type& node_key, const node_time_type& t);

  /**
   * :cell_key:       cellId to which the measurement belongs
   * :node_key:       nodeId of node which provided the measurement. Does not
   *                  have to be the original creator of measurement.
   * :measure_value:  timestamped
   *                  density count for cellId, provided by nodeId.
   */
  void update(const cell_key_type& cell_key, const node_key_type& node_key,
              node_mapped_type&& measure_value);

  void removeNeighbour(const node_key_type& neigbourId);
  bool hasNeighbour(const node_key_type& neigbourId) const;
  bool hasNeighbour(const node_key_type& neigbourId,
                    const cell_key_type& cellID) const;
  const int neighbourCount() const;
  void updateNeighbourCell(const node_key_type& neigbourId,
                           const cell_key_type& cellId);
  void clearLocalNodetoCellMap();

  std::shared_ptr<PositionMapView> getView(std::string view_name);
  std::shared_ptr<PositionMapView> getViewBySource(const node_key_type& nodeId);

  node_key_type getNodeId() const { return _localNodeId; }
  cell_key_type getCellId() const { return _currentCell; }

  void visit_entry(const entry_visitor_f& visitor) const;
  // remove selection string from all entries
  void clearEntrySelection();

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

    virtual cellEntryFiltered_r range() override;
  };

  class YmfView : public PositionMapView {
   public:
    YmfView(PositionMap<cell_key_type, cell_mapped_type, cell_ctor_type>* map)
        : PositionMapView(map, "ymf") {}

    virtual cellEntryFiltered_r range() override;
  };

  class SourceNodeView : public PositionMapView {
   public:
    SourceNodeView(
        PositionMap<cell_key_type, cell_mapped_type, cell_ctor_type>* map,
        node_key_type sourceNodeId)
        : PositionMapView(map, ""), sourceNodeId(sourceNodeId) {}

    virtual cellEntryFiltered_r range() override;

   protected:
    node_key_type sourceNodeId;
  };
};

} /* namespace rover */

// template implementations

#include "rover/common/positionMap/CellEntry.tcc"
#include "rover/common/positionMap/PositionMap.tcc"
#include "rover/common/positionMap/PositionMapView.tcc"
