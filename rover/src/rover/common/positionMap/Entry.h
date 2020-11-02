/*
 * Entry.h
 *
 *  Created on: Sep 9, 2020
 *      Author: sts
 */

#pragma once

#include <omnetpp/cexception.h>
#include <memory>
#include <set>
#include <sstream>
#include <string>

template <typename K, typename T>
class IEntry {
 public:
  using key_type = K;
  using time_type = T;
  IEntry();
  IEntry(const int, const time_type&, const time_type&);
  IEntry(const int);
  virtual ~IEntry() = default;
  virtual void reset(const time_type& t) {
    count = 0;
    _valid = false;
    measurement_time = t;
    received_time = t;
    selected_in = "";
  }
  virtual void clear(const time_type& t) {
    count = 0;
    measurement_time = t;
    received_time = t;
    selected_in = "";
  }
  const bool empty() const;
  virtual const bool valid() const { return _valid; }
  virtual void incrementCount(const time_type& t);
  virtual void decrementCount(const time_type& t);

  virtual const time_type& getMeasureTime() const;
  virtual const time_type& getReceivedTime() const;
  virtual const int getCount() const;

  virtual int compareMeasureTime(const IEntry& other) const;
  virtual int compareReceivedTime(const IEntry& other) const;

  virtual void setSource(const key_type& source);
  virtual const key_type& getSource() const;

  virtual void setSelectedIn(std::string viewName);
  virtual std::string getSelectedIn() const;

  virtual std::string csv(std::string delimiter) const;
  virtual std::string str() const;

 protected:
  int count;
  time_type measurement_time;
  time_type received_time;
  bool _valid;
  key_type source;
  std::string selected_in;
};

template <typename K, typename T>
class ILocalEntry : public IEntry<K, T> {
 public:
  virtual ~ILocalEntry() = default;
  ILocalEntry();
  ILocalEntry(const int, const T&, const T&);

  virtual void reset(const T& t) override;
  virtual void clear(const T& t) override;

  // STS: make protected.
  std::set<typename IEntry<K, T>::key_type> nodeIds;
};

template <typename K, typename T>
class EntryCtor {
 public:
  virtual ~EntryCtor() = default;
  virtual std::shared_ptr<IEntry<K, T>> entry() = 0;
  virtual std::shared_ptr<ILocalEntry<K, T>> localEntry() = 0;
  virtual std::shared_ptr<IEntry<K, T>> empty() = 0;
};

template <typename K, typename T>
class EntryDefaultCtorImpl : public EntryCtor<K, T> {
 public:
  std::shared_ptr<IEntry<K, T>> entry() override {
    return std::make_shared<IEntry<K, T>>();
  }

  std::shared_ptr<ILocalEntry<K, T>> localEntry() override {
    return std::make_shared<ILocalEntry<K, T>>();
  }

  std::shared_ptr<IEntry<K, T>> empty() override {
    return std::make_shared<IEntry<K, T>>(-1);
  }
};

/// implementation IEntry<K, T>

template <typename K, typename T>
inline IEntry<K, T>::IEntry()
    : count(0), measurement_time(), received_time(), _valid(false), source() {}

template <typename K, typename T>
inline IEntry<K, T>::IEntry(int count)
    : count(count),
      measurement_time(),
      received_time(),
      _valid(true),
      source() {}

template <typename K, typename T>
inline IEntry<K, T>::IEntry(const int count, const time_type& m_t,
                            const time_type& r_t)
    : count(count),
      measurement_time(m_t),
      received_time(r_t),
      _valid(true),
      source() {}

template <typename K, typename T>
inline const bool IEntry<K, T>::empty() const {
  return count < 0;
}

template <typename K, typename T>
inline void IEntry<K, T>::incrementCount(const time_type& t) {
  count++;
  measurement_time = t;
  received_time = t;
  _valid = true;
}

template <typename K, typename T>
inline void IEntry<K, T>::decrementCount(const time_type& t) {
  if (count <= 0) {
    throw omnetpp::cRuntimeError("Cell count decrement below 0.");
  }
  count--;
  _valid = true;
  measurement_time = t;
  received_time = t;
}

template <typename K, typename T>
inline const typename IEntry<K, T>::time_type& IEntry<K, T>::getMeasureTime()
    const {
  return measurement_time;
}

template <typename K, typename T>
inline const typename IEntry<K, T>::time_type& IEntry<K, T>::getReceivedTime()
    const {
  return received_time;
}

template <typename K, typename T>
inline const int IEntry<K, T>::getCount() const {
  return count;
}

template <typename K, typename T>
inline int IEntry<K, T>::compareMeasureTime(const IEntry& other) const {
  if (measurement_time == other.measurement_time) return 0;
  if (measurement_time < other.measurement_time) {
    return -1;
  } else {
    return 1;
  }
}

template <typename K, typename T>
inline int IEntry<K, T>::compareReceivedTime(const IEntry& other) const {
  if (received_time == other.received_time) return 0;
  if (received_time < other.received_time) {
    return -1;
  } else {
    return 1;
  }
}

template <typename K, typename T>
inline void IEntry<K, T>::setSource(const key_type& source) {
  this->source = source;
}

template <typename K, typename T>
inline const K& IEntry<K, T>::getSource() const {
  return this->source;
}

template <typename K, typename T>
void IEntry<K, T>::setSelectedIn(std::string viewName) {
  this->selected_in = viewName;
}

template <typename K, typename T>
std::string IEntry<K, T>::getSelectedIn() const {
  return this->selected_in;
}

template <typename K, typename T>
inline std::string IEntry<K, T>::csv(std::string delimiter) const {
  std::stringstream out;
  out << this->count << delimiter << this->measurement_time << delimiter
      << this->received_time << delimiter << this->source << delimiter
      << this->selected_in;
  return out.str();
}

template <typename K, typename T>
inline std::string IEntry<K, T>::str() const {
  std::stringstream os;
  os << "Count: " << this->count
     << "| measurement_time:" << this->measurement_time
     << "| received_time: " << this->received_time
     << "| valid: " << this->valid();
  return os.str();
}

///////////////////////////////////////////////////

template <typename K, typename T>
inline ILocalEntry<K, T>::ILocalEntry() : IEntry<K, T>() {}

template <typename K, typename T>
inline ILocalEntry<K, T>::ILocalEntry(const int count, const T& m_t,
                                      const T& r_t)
    : IEntry<K, T>(count, m_t, r_t) {}

template <typename K, typename T>
inline void ILocalEntry<K, T>::reset(const T& t) {
  IEntry<K, T>::reset(t);
  this->nodeIds.clear();
}

template <typename K, typename T>
inline void ILocalEntry<K, T>::clear(const T& t) {
  IEntry<K, T>::clear(t);
  this->nodeIds.clear();
}
