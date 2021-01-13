/*
 * IDensityMap.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sts
 */

#pragma once

#include <memory>
#include "rover/common/util/FileWriter.h"

namespace rover {

template <typename GRID>
class IDensityMapHandler {
 public:
  virtual ~IDensityMapHandler() = default;
  virtual void updateLocalMap() = 0;
  virtual void computeValues() = 0;
  // FIXME: allow global fileWriter object to pass in.
  virtual void writeMap() = 0;
  virtual std::shared_ptr<GRID> getMap() = 0;
  //  FIXME: make mergeMap independent from Packet (see ArteryDensityMapApp.cc)
  //  virtual void mergeMap(const GRID map) = 0;
};

}  // namespace rover
