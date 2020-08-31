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
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <map>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace rover {

template <typename T>
class IEntry {
 public:
  using time_type = T;
  virtual void reset() = 0;
  virtual const bool valid() const = 0;
  virtual void incrementCount(const time_type& t) = 0;
};

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

  void updateLocalMeasure(mapped_type& m) { local() = m; }

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

 private:
  using map_type = std::map<key_type, mapped_type>;
  map_type _data;
  key_type _localKey;
  entry_ctor _entryCtor;
  mapped_type* _localEntry = nullptr;
};

template <typename VALUE, typename ENTRY_KEY>
class PositionMapDefaultCtor {
 public:
  PositionMapDefaultCtor(ENTRY_KEY k) : _k(k) {}
  VALUE operator()() { return VALUE(_k); }

 private:
  ENTRY_KEY _k;
};

template <typename CELL_KEY, typename VALUE,
          typename CTOR =
              PositionMapDefaultCtor<VALUE, typename VALUE::key_type>>
class PositionMap {
 public:
  using key_type = CELL_KEY;  // key for one cell or triangle (bucket)
  using mapped_type = VALUE;  // container in each bucket.
  using value_type = std::pair<const key_type&, mapped_type&>;
  using creator_type = CTOR;  // creator for container

  // each bucket contains an entry map
  using entry_key_type = typename VALUE::key_type;
  using entry_mapped_type = typename VALUE::mapped_type;
  using entry_value_type = typename VALUE::value_type;

  // map one entry_mapped_type instance  to one key_type instance
  // used by boost::iterator_ranges to filter/aggregate the correct
  // entry_mapped_type based on given predicate/transformer.
  using view_value_type = std::pair<const key_type&, entry_mapped_type&>;

 private:
  using map_type = std::map<key_type, mapped_type>;
  map_type _map;
  // identifier that maps to local entry_mapped_type value
  entry_key_type _localId;
  creator_type _cellCtor;
  // range of map_type::value_type items
  using data_range = boost::iterator_range<typename map_type::iterator>;

 public:
  using data_filter = std::function<bool(const typename map_type::value_type&)>;
  // range of all valid map_type::value_type elements
  using data_map_range = const boost::filtered_range<data_filter, data_range>;

  using entry_filter = std::function<bool(const entry_mapped_type&)>;
  using entry_transform =
      std::function<view_value_type(typename map_type::value_type&)>;

  using entry_map_range = boost::transformed_range<
      entry_transform, const boost::filtered_range<data_filter, data_range>>;

 public:
  virtual ~PositionMap() = default;

  PositionMap(entry_key_type localId) : _localId(localId), _cellCtor(localId) {}

  PositionMap(entry_key_type localId, creator_type&& ctor)
      : _localId(localId), _cellCtor(ctor) {}

  void resetLocalMap() {
    for (auto& entry : _map) {
      entry.second.resetLocalMeasure();
    }
  }

  mapped_type& getCellEntry(const key_type& cell_key) {
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

  void updateLocal(const key_type& cell_key, entry_mapped_type& measure_value) {
    getCellEntry(cell_key).local() = measure_value;
  }

  void incrementLocal(const key_type& cell_key, const omnetpp::simtime_t& t) {
    getCellEntry(cell_key).local().incrementCount(t);
  }

  entry_map_range localMap() {
    entry_transform _t = [](typename map_type::value_type& map_value) {
      view_value_type ret =
          view_value_type{map_value.first, map_value.second.local()};
      return ret;
    };

    data_filter _f = [](const typename map_type::value_type& map_value) {
      auto& value = map_value.second;
      return value.hasValidLocalMeasure();
    };

    using namespace boost::adaptors;
    data_range range_all = boost::make_iterator_range(_map.begin(), _map.end());

    return range_all | filtered(_f) | transformed(_t);
  }

  void printLocalMap() {
    using namespace omnetpp;
    for (auto entry : localMap()) {
      EV_DEBUG << "   Cell(" << entry.first.first << ", " << entry.first.second
               << ") " << entry.second << std::endl;
    }
  }

 private:
};

} /* namespace rover */
