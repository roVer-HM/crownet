
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
#include <boost/range/algorithm/max_element.hpp>
#include <boost/range/algorithm/min_element.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <map>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace rover {

template <typename T>
class IEntry {
 public:
  using time_type = T;
  IEntry();
  IEntry(int, time_type&, time_type&);
  virtual void reset() {
    count = 0;
    _valid = false;
  }
  virtual const bool valid() const { return _valid; }
  virtual void incrementCount(const time_type& t);

  virtual const time_type& getMeasureTime() const;
  virtual const time_type& getReceivedTime() const;
  virtual const int getCount() const;

  virtual int compareMeasureTime(const IEntry& other) const;
  virtual int compareReceivedTime(const IEntry& other) const;

  virtual std::string csv(std::string delimiter) const = 0;
  virtual std::string str() const = 0;

 protected:
  int count;
  time_type measurement_time;
  time_type received_time;
  bool _valid;
};

template <typename T>
IEntry<T>::IEntry()
    : count(0), measurement_time(), received_time(), _valid(false) {}

template <typename T>
IEntry<T>::IEntry(int count, time_type& m_t, time_type& r_t)
    : count(count), measurement_time(m_t), received_time(r_t), _valid(true) {}

template <typename T>
void IEntry<T>::incrementCount(const time_type& t) {
  count++;
  measurement_time = t;
  received_time = t;
  _valid = true;
}

template <typename T>
const typename IEntry<T>::time_type& IEntry<T>::getMeasureTime() const {
  return measurement_time;
}

template <typename T>
const typename IEntry<T>::time_type& IEntry<T>::getReceivedTime() const {
  return received_time;
}

template <typename T>
const int IEntry<T>::getCount() const {
  return count;
}

template <typename T>
int IEntry<T>::compareMeasureTime(const IEntry& other) const {
  if (measurement_time == other.measurement_time) return 0;
  if (measurement_time < other.measurement_time) {
    return -1;
  } else {
    return 1;
  }
}

template <typename T>
int IEntry<T>::compareReceivedTime(const IEntry& other) const {
  if (received_time == other.received_time) return 0;
  if (received_time < other.received_time) {
    return -1;
  } else {
    return 1;
  }
}

template <typename VALUE>
class EntryDefaultCtor {
 public:
  VALUE operator()() { return VALUE(); }
};

template <
    typename KEY, typename VALUE,
    typename std::enable_if<std::is_base_of<IEntry<typename VALUE::time_type>,
                                            VALUE>::value>::type* = nullptr,
    typename ENTRY_CTOR = EntryDefaultCtor<VALUE>>
class CellEntry {
 public:
  using key_type = KEY;
  using mapped_type = VALUE;
  using value_type = std::pair<const key_type&, mapped_type&>;
  using entry_ctor = ENTRY_CTOR;

 private:
  using map_type = std::map<key_type, mapped_type>;
  map_type _data;
  key_type _localKey;
  entry_ctor _entryCtor;
  mapped_type* _localEntry = nullptr;

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
  CellEntry(
      key_type localKey,
      typename std::enable_if<std::is_default_constructible<T>::value>::type* =
          nullptr)
      : _localKey(localKey) {}

  CellEntry(key_type localKey, entry_ctor&& ctor)
      : _localKey(localKey), _entryCtor(std::move(ctor)) {}

  const bool hasLocalMeasure() const { return _localEntry != nullptr; }
  const bool hasValidLocalMeasure() const {
    return hasLocalMeasure() && local().valid();
  }

  void resetLocalMeasure() {
    if (hasLocalMeasure()) _localEntry->reset();
  }

  const key_type localKey() const { return _localKey; }

  mapped_type& local() {
    if (!hasLocalMeasure()) {
      auto ret = _data.emplace(std::piecewise_construct,
                               std::forward_as_tuple(_localKey),
                               std::forward_as_tuple(_entryCtor()));
      if (!ret.second) {
        throw omnetpp::cRuntimeError("element with key %s already existed.");
      }
      _localEntry = &(ret.first->second);  // address of mapped_type in map.
    }

    return *_localEntry;
  }
  const mapped_type& local() const {
    return const_cast<CellEntry*>(this)->local();
  }

  mapped_type& get(const key_type& key) {
    auto measure = _data.find(key);
    if (measure != _data.end()) {
      return measure->second;
    } else {
      auto newMeasure =
          _data.emplace(std::piecewise_construct, std::forward_as_tuple(key),
                        std::forward_as_tuple(_entryCtor()));
      if (!newMeasure.second) {
        throw omnetpp::cRuntimeError("error inserting newMeasure");
      } else {
        return newMeasure.first->second;
      }
    }
  }
  const mapped_type& get() const { return const_cast<CellEntry*>(this)->get(); }

  mapped_type& youngestMeasureFirst(bool prefereLocal = true) {
    map_binary_filter _f = [](const typename map_type::value_type lhs,
                              const typename map_type::value_type rhs) -> bool {
      // Comparator true if lhs lessThan rhs
      return lhs.second.compareMeasureTime(rhs.second) < 0;
    };
    auto range = validRange();
    //    auto ret = boost::range::min_element(rangge, _f);
    auto ret = boost::range::max_element(range, _f);

    std::string s = str();

    if (prefereLocal && hasValidLocalMeasure() &&
        ret->second.compareMeasureTime(*_localEntry) == 0) {
      // return local measure if it has the same age.
      return *_localEntry;
    } else {
      return ret->second;
    }
  }
  const mapped_type& youngestMeasureFirst() const {
    return const_cast<CellEntry*>(this)->youngestMeasureFirst();
  }

  const bool hasValid() const { return boost::size(validRange()); }

  map_range_filterd validRange() {
    map_unary_filter _f = [](const typename map_type::value_type val) -> bool {
      return val.second.valid();
    };
    using namespace boost::adaptors;
    map_range rng = boost::make_iterator_range(_data.begin(), _data.end());
    return rng | filtered(_f);
  }
  const map_range_filterd validRange() const {
    return const_cast<CellEntry*>(this)->validRange();
  }

  std::string str() const {
    std::ostringstream s;
    for (const auto entry : _data) {
      s << entry.second << "\n";
    }
    return s.str();
  }
};

template <typename VALUE, typename ENTRY_KEY>
class CellCtor {
 public:
  CellCtor(ENTRY_KEY k) : _k(k) {}
  VALUE operator()() { return VALUE(_k); }

 private:
  ENTRY_KEY _k;
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

  // each bucket contains an entry map
  using creator_type = CTOR;  // creator for container
  using node_key_type = typename VALUE::key_type;
  using node_mapped_type = typename VALUE::mapped_type;
  using node_value_type = typename VALUE::value_type;

  // map one entry_mapped_type instance  to one key_type instance
  // used by boost::iterator_ranges to filter/aggregate the correct
  // entry_mapped_type based on given predicate/transformer.
  using view_value_type = std::pair<const cell_key_type&, node_mapped_type&>;

 private:
  using map_type = std::map<cell_key_type, cell_mapped_type>;
  using map_views = std::map<std::string, std::shared_ptr<PositionMapView>>;
  map_type _map;
  map_views views;

  // identifier that maps to local entry_mapped_type value
  node_key_type _localNodeId;  // NodeId of this node.
  cell_key_type _currentCell;  // CellId this node  currently ocupies.
  creator_type _cellCtor;

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

 public:
  virtual ~PositionMap() = default;

  PositionMap(node_key_type localId)
      : _localNodeId(localId), _cellCtor(localId) {
    initViews();
  }

  PositionMap(node_key_type localId, creator_type&& ctor)
      : _localNodeId(localId), _cellCtor(ctor) {
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

  void resetLocalMap() {
    for (auto& entry : _map) {
      entry.second.resetLocalMeasure();
    }
  }

  virtual void incrementLocal(const cell_key_type& cell_key,
                              const omnetpp::simtime_t& t,
                              bool ownPosition = false) {
    getCellEntry(cell_key).local().incrementCount(t);
    if (ownPosition) {
      _currentCell = cell_key;
    }
  }

  void update(const cell_key_type& cell_key, const node_key_type& node_key,
              node_mapped_type& measure_value) {
    getCellEntry(cell_key).get(node_key) = measure_value;
  }

  std::shared_ptr<PositionMapView> getView(std::string view_name) {
    auto iter = views.find(view_name);
    if (iter == views.end())
      omnetpp::cRuntimeError("View '%' not found ", view_name.c_str());

    return iter->second;
  }

  node_key_type getNodeIde() { return _localNodeId; }

  class PositionMapView {
   public:
    PositionMapView() : _cell_map(nullptr), _view_name("") {}
    PositionMapView(
        PositionMap<cell_key_type, cell_mapped_type, creator_type>* map,
        std::string view_name)
        : _cell_map(map), _view_name(view_name) {}

   public:
    virtual ~PositionMapView() = default;
    virtual cellEntryFiltered_r range() { throw omnetpp::cRuntimeError("Err"); }

    const cellEntryFiltered_r range() const {
      return const_cast<PositionMapView*>(this)->range();
    }

    std::string str() {
      std::stringstream s;
      s << "Map[ " << _view_name << "] (NodeId: " << _cell_map->_localNodeId
        << "\n";
      for (auto entry : range()) {
        s << "   Cell(" << entry.first.first << ", " << entry.first.second
          << ") " << entry.second << std::endl;
      }
      return s.str();
    }

    int size() const { return boost::size(range()); }

    node_key_type getId() { return _cell_map->_localNodeId; }

    void print() {
      using namespace omnetpp;
      EV_DEBUG << str();
    }

   protected:
    PositionMap<cell_key_type, cell_mapped_type, creator_type>* _cell_map;
    std::string _view_name;
  };

  class LocalView : public PositionMapView {
   public:
    LocalView(PositionMap<cell_key_type, cell_mapped_type, creator_type>* map)
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
            view_value_type{map_value.first, map_value.second.local()};
        return ret;
      };

      using namespace boost::adaptors;
      cellContainer_r range_all = this->_cell_map->range();

      return range_all | filtered(_f) | transformed(_t);
    }
  };

  class YmfView : public PositionMapView {
   public:
    YmfView(PositionMap<cell_key_type, cell_mapped_type, creator_type>* map)
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
};

} /* namespace rover */
