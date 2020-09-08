/*
 * GridDensityMap.cc
 *
 *  Created on: Aug 25, 2020
 *      Author: sts
 */

#include "GridDensityMap.h"

namespace rover {

DensityMeasure::DensityMeasure() : IEntry<omnetpp::simtime_t>() {}
DensityMeasure::DensityMeasure(int count, omnetpp::simtime_t& measurement_time,
                               omnetpp::simtime_t& received_time)
    : IEntry<omnetpp::simtime_t>(count, measurement_time, received_time) {}

std::string DensityMeasure::csv(std::string delimiter) const {
  std::stringstream out;
  out << count << delimiter << measurement_time.dbl() << delimiter
      << received_time.dbl();
  return out.str();
}

std::string DensityMeasure::str() const {
  std::stringstream os;
  os << "Count: " << count << "| measurement_time:" << measurement_time.dbl()
     << "| received_time: " << received_time.dbl() << "| valid: " << valid();
  return os.str();
}

RegularGridMap::RegularGridMap(NodeId id, double gridSize)
    : PositionMap(id), gridSize(gridSize) {}

void RegularGridMap::incrementLocal(const inet::Coord& coord,
                                    const omnetpp::simtime_t& t,
                                    bool ownPosition) {
  CellId id =
      std::make_pair(floor(coord.x / gridSize), floor(coord.y / gridSize));
  PositionMap::incrementLocal(id, t, ownPosition);
}

} /* namespace rover */
