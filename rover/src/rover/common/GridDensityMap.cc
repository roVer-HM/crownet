/*
 * GridDensityMap.cc
 *
 *  Created on: Aug 25, 2020
 *      Author: sts
 */

#include "GridDensityMap.h"

namespace rover {

DensityMeasure::DensityMeasure()
    : count(0), measurement_time(0.0), received_time(0.0), _valid(false) {}
DensityMeasure::DensityMeasure(int count, omnetpp::simtime_t& measurement_time,
                               omnetpp::simtime_t& received_time)
    : count(count),
      measurement_time(measurement_time),
      received_time(received_time),
      _valid(true) {}

const bool DensityMeasure::valid() const { return _valid; }

void DensityMeasure::reset() {
  count = 0;
  _valid = false;
}

void DensityMeasure::incrementCount(const omnetpp::simtime_t& t) {
  count++;
  measurement_time = t;
  received_time = t;
  _valid = true;
}

std::ostream& operator<<(std::ostream& os, const DensityMeasure& obj) {
  os << "Count: " << obj.count
     << "| measurement_time:" << obj.measurement_time.dbl()
     << "| received_time: " << obj.received_time.dbl()
     << "| valid: " << obj.valid();
  return os;
}

} /* namespace rover */
