/*
 * GridDensityMap.h
 *
 *  Created on: Aug 25, 2020
 *      Author: sts
 */

#pragma once

#include <math.h>
#include <omnetpp/simtime_t.h>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/map.hpp>
#include <map>
#include <unordered_map>
#include "inet/common/geometry/common/Coord.h"
#include "rover/common/PositionMap.h"

namespace rover {

class DensityMeasure : public IEntry<omnetpp::simtime_t> {
 public:
  DensityMeasure();
  DensityMeasure(int, omnetpp::simtime_t& measurement_time,
                 omnetpp::simtime_t& received_time);

  virtual std::string csv(std::string delimiter) const override;
  virtual std::string str() const override;
};

using CellId = std::pair<int, int>;
using NodeId = std::string;

class RegularGridMap
    : public PositionMap<CellId, CellEntry<NodeId, DensityMeasure>> {
 public:
  RegularGridMap(NodeId id, double gridSize);
  virtual ~RegularGridMap() = default;

  using PositionMap::incrementLocal;
  virtual void incrementLocal(const inet::Coord& coord,
                              const omnetpp::simtime_t& t,
                              bool ownPosition = false);

 private:
  double gridSize;
};

} /* namespace rover */
