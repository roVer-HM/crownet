/*
 * DensityMeasure.h
 *
 *  Created on: Sep 9, 2020
 *      Author: sts
 */

#pragma once

#include <omnetpp.h>
#include "rover/common/positionMap/Entry.h"

template <typename NODE_ID>
class DensityMeasure : public IEntry<NODE_ID, omnetpp::simtime_t> {
 public:
  virtual ~DensityMeasure() = default;
  DensityMeasure() : IEntry<NODE_ID, omnetpp::simtime_t>() {}
  DensityMeasure(const int count)
      : IEntry<NODE_ID, omnetpp::simtime_t>(count) {}
  DensityMeasure(const int count, const omnetpp::simtime_t& measurement_time,
                 const omnetpp::simtime_t& received_time)
      : IEntry<NODE_ID, omnetpp::simtime_t>(count, measurement_time,
                                            received_time) {}

  virtual std::string csv(std::string delimiter) const override;
  virtual std::string str() const override;
};

template <typename NODE_ID>
class LocalDensityMeasure : public DensityMeasure<NODE_ID> {
 public:
  LocalDensityMeasure();
  LocalDensityMeasure(int count, const omnetpp::simtime_t& measurement_time,
                      const omnetpp::simtime_t& received_time);

  virtual void reset() override;

  std::set<typename LocalDensityMeasure<NODE_ID>::key_type>
      nodeIds;  // ids of nodes which make up the count
};

template <typename NODE_ID>
class DensityMeasureCtor : public EntryCtor<DensityMeasure<NODE_ID>> {
 public:
  std::shared_ptr<DensityMeasure<NODE_ID>> entry() override;
  std::shared_ptr<DensityMeasure<NODE_ID>> localEntry() override;
  std::shared_ptr<DensityMeasure<NODE_ID>> empty() const override;

 private:
  std::shared_ptr<DensityMeasure<NODE_ID>> _empty =
      std::make_shared<DensityMeasure<NODE_ID>>(-1);
};

/// implementation DensityMeasureCtor<NODE_ID>
template <typename NODE_ID>
inline std::shared_ptr<DensityMeasure<NODE_ID>>
DensityMeasureCtor<NODE_ID>::entry() {
  return std::make_shared<DensityMeasure<NODE_ID>>();
}

template <typename NODE_ID>
inline std::shared_ptr<DensityMeasure<NODE_ID>>
DensityMeasureCtor<NODE_ID>::localEntry() {
  return std::make_shared<LocalDensityMeasure<NODE_ID>>();
}

template <typename NODE_ID>
inline std::shared_ptr<DensityMeasure<NODE_ID>>
DensityMeasureCtor<NODE_ID>::empty() const {
  return _empty;
}

/// implementation DensityMeasure<NODE_ID>
template <typename NODE_ID>
inline std::string DensityMeasure<NODE_ID>::csv(std::string delimiter) const {
  std::stringstream out;
  out << this->count << delimiter << this->measurement_time.dbl() << delimiter
      << this->received_time.dbl() << delimiter << this->source;
  return out.str();
}

template <typename NODE_ID>
inline std::string DensityMeasure<NODE_ID>::str() const {
  std::stringstream os;
  os << "Count: " << this->count
     << "| measurement_time:" << this->measurement_time.dbl()
     << "| received_time: " << this->received_time.dbl()
     << "| valid: " << this->valid();
  return os.str();
}

/// implementation LocalDensityMeasure<NODE_ID>

template <typename NODE_ID>
LocalDensityMeasure<NODE_ID>::LocalDensityMeasure()
    : DensityMeasure<NODE_ID>() {}

template <typename NODE_ID>
LocalDensityMeasure<NODE_ID>::LocalDensityMeasure(
    int count, const omnetpp::simtime_t& measurement_time,
    const omnetpp::simtime_t& received_time)
    : DensityMeasure<NODE_ID>(count, measurement_time, received_time) {}

template <typename NODE_ID>
inline void LocalDensityMeasure<NODE_ID>::reset() {
  this->count = 0;
  this->_valid = false;
  this->nodeIds.clear();
}
