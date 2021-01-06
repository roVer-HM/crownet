/*
 * PositionHistoryProvider.h
 *
 *  Created on: Aug 17, 2020
 *      Author: sts
 */

#pragma once

#include <omnetpp.h>
#include "rover/common/util/rover_util.h"
#include "rover/rover.h"

namespace rover {

class IPositionHistoryProvider {
 public:
  virtual ~IPositionHistoryProvider() = default;

  virtual void recoredTimeCoord(omnetpp::simtime_t time, inet::Coord coord) = 0;

  virtual std::vector<PathPoint> getPositionHistory() = 0;
  virtual std::vector<PathPoint> getDeltaPositionHistory() = 0;
  virtual int historySize() = 0;

  virtual const inet::Coord& getCurrentVelocity() = 0;
  virtual const inet::Coord& getCurrentPosition() = 0;
};

} /* namespace rover */
