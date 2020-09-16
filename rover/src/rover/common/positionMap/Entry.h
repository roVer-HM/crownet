/*
 * Entry.h
 *
 *  Created on: Sep 9, 2020
 *      Author: sts
 */

#pragma once

#include <memory>
#include <set>
#include <sstream>
#include <string>

template <typename VALUE>
class EntryCtor {
 public:
  virtual ~EntryCtor() = default;
  virtual std::shared_ptr<VALUE> entry() = 0;
  virtual std::shared_ptr<VALUE> localEntry() = 0;
};

template <typename VALUE>
class EntryDefaultCtorImpl : public EntryCtor<VALUE> {
 public:
  std::shared_ptr<VALUE> entry() override { return std::make_shared<VALUE>(); }

  std::shared_ptr<VALUE> localEntry() override {
    return std::make_shared<VALUE>();
  }
};

template <typename K, typename T>
class IEntry {
 public:
  using key_type = K;
  using time_type = T;
  IEntry();
  IEntry(const int, const time_type&, const time_type&);
  virtual ~IEntry() = default;
  virtual void reset() {
    count = 0;
    _valid = false;
  }
  const bool empty() const;
  virtual const bool valid() const { return _valid; }
  virtual void incrementCount(const time_type& t);

  virtual const time_type& getMeasureTime() const;
  virtual const time_type& getReceivedTime() const;
  virtual const int getCount() const;

  virtual int compareMeasureTime(const IEntry& other) const;
  virtual int compareReceivedTime(const IEntry& other) const;

  virtual std::string csv(std::string delimiter) const;
  virtual std::string str() const;

 protected:
  int count;
  time_type measurement_time;
  time_type received_time;
  bool _valid;
};

/// implementation IEntry<K, T>

template <typename K, typename T>
inline IEntry<K, T>::IEntry()
    : count(0), measurement_time(), received_time(), _valid(false) {}

template <typename K, typename T>
inline IEntry<K, T>::IEntry(const int count, const time_type& m_t,
                            const time_type& r_t)
    : count(count), measurement_time(m_t), received_time(r_t), _valid(true) {}

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
inline std::string IEntry<K, T>::csv(std::string delimiter) const {
  std::stringstream out;
  out << this->count << delimiter << this->measurement_time << delimiter
      << this->received_time;
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
