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

using CellId = std::pair<int, int>;
using NodeId = std::string;

class DensityMeasure : public IEntry<NodeId, omnetpp::simtime_t> {
 public:
  virtual ~DensityMeasure() = default;
  DensityMeasure();
  DensityMeasure(int, omnetpp::simtime_t& measurement_time,
                 omnetpp::simtime_t& received_time);

  virtual std::string csv(std::string delimiter) const override;
  virtual std::string str() const override;
};

class LocalDensityMeasure : public DensityMeasure {
 public:
  LocalDensityMeasure();
  LocalDensityMeasure(int, omnetpp::simtime_t& measurement_time,
                      omnetpp::simtime_t& received_time);

  std::set<LocalDensityMeasure::key_type>
      nodeIds;  // ids of nodes which make up the count
};

class DensityMeasureCtor : public EntryCtor<DensityMeasure> {
 public:
  std::shared_ptr<DensityMeasure> entry() override {
    return std::make_shared<DensityMeasure>();
  }

  std::shared_ptr<DensityMeasure> localEntry() override {
    return std::make_shared<LocalDensityMeasure>();
  }
};

class RegularGridMap
    : public PositionMap<CellId,
                         CellEntry<DensityMeasure, DensityMeasureCtor>> {
 public:
  RegularGridMap(NodeId id, double gridSize);
  virtual ~RegularGridMap() = default;

  using PositionMap::incrementLocal;
  virtual void incrementLocal(const inet::Coord& coord, const NodeId nodeId,
                              const omnetpp::simtime_t& t,
                              bool ownPosition = false);

 private:
  double gridSize;
};

} /* namespace rover */
