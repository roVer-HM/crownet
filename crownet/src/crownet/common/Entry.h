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

#include "crownet/common/util/FilePrinter.h"

struct EntryDist{
    double sourceOwner;
    double sourceEntry;
    double ownerEntry;
};

template <typename K, typename T>
class IEntry : public crownet::FilePrinter {
 public:
  using key_type = K;
  using time_type = T;
  IEntry();
  IEntry(const double, const time_type&, const time_type&);
  IEntry(const double, const time_type&, const time_type&, const key_type& source);
  IEntry(const double);
  virtual ~IEntry() = default;
  virtual void reset(const time_type& t) {
    count = 0;
    _valid = false;
    measurement_time = t;
    received_time = t;
    selected_in = "";
  }
  virtual void reset() {
    count = 0;
    _valid = false;
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
  virtual void touch(const time_type& t);  // update time only

  virtual const time_type& getMeasureTime() const;
  virtual const time_type& getReceivedTime() const;
  virtual const int getCount() const;

  virtual void setCount(double count);

  virtual int compareMeasureTime(const IEntry& other) const;
  virtual int compareReceivedTime(const IEntry& other) const;

  virtual void setSource(const key_type& source);
  virtual const key_type& getSource() const;

  virtual void setSelectedIn(std::string viewName);
  virtual std::string getSelectedIn() const;

  virtual std::string csv(std::string delimiter) const;
  virtual std::string str() const;

  virtual int columns() const override;
  virtual void writeTo(std::ostream& out,
                       const std::string& sep) const override;
  virtual void writeHeaderTo(std::ostream& out,
                             const std::string& sep) const override;

  //  bool operator<(const Cell<C, N, T>& rhs) const;
  //  bool operator>(const Cell<C, N, T>& rhs) const;
  bool operator==(const IEntry<K, T>& rhs) const;

 protected:
  double count;
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
  ILocalEntry(const int);

  virtual void reset(const T& t) override;
  virtual void clear(const T& t) override;
  virtual std::string str() const override;
  virtual std::string nodeString(const std::string& sep) const;

  // STS: make protected.
  std::set<typename IEntry<K, T>::key_type> nodeIds;
};

template <typename K, typename T>
class EntryCtor {
 public:
  virtual ~EntryCtor() = default;
  virtual std::shared_ptr<IEntry<K, T>> entry() const = 0;
  virtual std::shared_ptr<ILocalEntry<K, T>> localEntry() const = 0;
  virtual std::shared_ptr<IEntry<K, T>> empty() const = 0;
};

template <typename K, typename T>
class EntryDefaultCtorImpl : public EntryCtor<K, T> {
 public:
  std::shared_ptr<IEntry<K, T>> entry() const override{
    return std::make_shared<IEntry<K, T>>();
  }

  std::shared_ptr<ILocalEntry<K, T>> localEntry() const override {
    return std::make_shared<ILocalEntry<K, T>>();
  }

  std::shared_ptr<IEntry<K, T>> empty() const override {
    return std::make_shared<IEntry<K, T>>(-1);
  }
};

/// implementation IEntry<K, T>

template <typename K, typename T>
inline IEntry<K, T>::IEntry()
    : count(0), measurement_time(), received_time(), _valid(true), source() {}

template <typename K, typename T>
inline IEntry<K, T>::IEntry(double count)
    : count(count),
      measurement_time(),
      received_time(),
      _valid(count >= 0),
      source() {}

template <typename K, typename T>
inline IEntry<K, T>::IEntry(const double count, const time_type& m_t,
                            const time_type& r_t)
    : count(count),
      measurement_time(m_t),
      received_time(r_t),
      _valid(true),
      source(),
      entryDist(){}

template <typename K, typename T>
inline IEntry<K, T>::IEntry(const double count, const time_type& m_t,
                            const time_type& r_t, const key_type& source, const EntryDist& dist)
    : count(count),
      measurement_time(m_t),
      received_time(r_t),
      _valid(true),
      source(source),
      entryDist(dist){}

template <typename K, typename T>
inline IEntry<K, T>::IEntry(const double count, const time_type& m_t, const time_type& r_t,
        const key_type&& source, const EntryDist&& dist)
    : count(count),
      measurement_time(m_t),
      received_time(r_t),
      _valid(true),
      source(std::move(source)),
      entryDist(std::move(dist)){}


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
void IEntry<K, T>::touch(const time_type& t) {
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
inline  void IEntry<K, T>::setCount(double count){
    this->count = count;
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
  os << "Count: " << this->count << "| meas_t: " << this->measurement_time
     << "| recv_t: " << this->received_time << "| valid: " << this->valid();
  return os.str();
}

template <typename K, typename T>
int IEntry<K, T>::columns() const {
  return 5;
}

template <typename K, typename T>
void IEntry<K, T>::writeTo(std::ostream& out, const std::string& sep) const {
  out << this->count << sep << this->measurement_time << sep
      << this->received_time << sep << this->source << sep << this->selected_in;
}

template <typename K, typename T>
void IEntry<K, T>::writeHeaderTo(std::ostream& out,
                                 const std::string& sep) const {
  out << "count" << sep << "measured_t" << sep << "received_t" << sep
      << "source" << sep << "selection";
}

template <typename K, typename T>
bool IEntry<K, T>::operator==(const IEntry<K, T>& rhs) const {
  if (this == &rhs) return true;
  return (this->count == rhs.count) && (this->source == rhs.source) &&
         (this->measurement_time == rhs.measurement_time) &&
         (this->received_time == rhs.received_time) &&
         (this->_valid == rhs._valid);
}

///////////////////////////////////////////////////

template <typename K, typename T>
inline ILocalEntry<K, T>::ILocalEntry() : IEntry<K, T>() {}

template <typename K, typename T>
inline ILocalEntry<K, T>::ILocalEntry(const int count, const T& m_t,
                                      const T& r_t)
    : IEntry<K, T>(count, m_t, r_t) {}

template <typename K, typename T>
inline ILocalEntry<K, T>::ILocalEntry(const int count) : IEntry<K, T>(count) {}

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

template <typename K, typename T>
inline std::string ILocalEntry<K, T>::str() const {
  std::stringstream os;
  os << IEntry<K, T>::str() << "| node_ids: {";
  int nCount = this->nodeIds.size() - 1;
  for (const auto& e : this->nodeIds) {
    os << e;
    if (nCount > 0) {
      os << ", ";
      --nCount;
    }
  }
  os << "}";
  return os.str();
}

template <typename K, typename T>
std::string ILocalEntry<K, T>::nodeString(const std::string& sep) const {
  std::stringstream os;
  int nCount = this->nodeIds.size() - 1;
  for (const auto& e : this->nodeIds) {
    os << e;
    if (nCount-- > 0) {
      os << ", ";
    }
  }
  return os.str();
}
