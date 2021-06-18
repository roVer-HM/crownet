/*
 * PositionHistoryProvider.h
 *
 *  Created on: Aug 17, 2020
 *      Author: sts
 */

#pragma once

#include <omnetpp.h>
#include "crownet/common/util/crownet_util.h"
#include "crownet/crownet.h"

namespace crownet {

class IPositionHistoryProvider {
 public:
  virtual ~IPositionHistoryProvider() = default;

  virtual void recoredTimeCoord(const omnetpp::simtime_t& time, const inet::Coord& coord) = 0;

  virtual std::vector<PathPoint> getPositionHistory() = 0;
  virtual std::vector<PathPoint> getDeltaPositionHistory() = 0;
  virtual int historySize() = 0;

  virtual const inet::Coord& getCurrentVelocity() = 0;
  virtual const inet::Coord& getCurrentPosition() = 0;
};

} /* namespace crownet */
