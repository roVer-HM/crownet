//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#pragma once

#include "../common/util/rover_util.h"
#include "IPositionHistoryProvider.h"
#include "rover/rover.h"

#include "veins_inet/vadere/VaderePersonMobility.h"

namespace rover {

class VaderePersonTracedMobility : public veins::VaderePersonMobility,
                                   public IPositionHistoryProvider {
 public:
  VaderePersonTracedMobility(){};
  virtual ~VaderePersonTracedMobility(){};

  virtual void initialize(int stage) override;  // cModule

  /** @brief called by class VeinsInetManager */
  virtual void preInitialize(
      std::shared_ptr<IMobileAgent> mobileAgent) override;

  /** @brief called by class VeinsInetManager */
  virtual void nextPosition(std::shared_ptr<IMobileAgent> mobileAgent) override;

  virtual void moveAndUpdate() override;

  virtual void recoredTimeCoord(simtime_t time, inet::Coord coord) override;

  virtual std::vector<PathPoint> getPositionHistory() override;
  virtual std::vector<PathPoint> getDeltaPositionHistory() override;
  virtual int historySize() override;

  virtual inet::Coord getCurrentPosition() override {
    return veins::VaderePersonMobility::getCurrentPosition();
  }  // MobilityBase
  virtual inet::Coord getCurrentVelocity() override {
    return veins::VaderePersonMobility::getCurrentVelocity();  // MobilityBase
  }

 private:
  simtime_t recordThreshold;
  simtime_t lastRecordedTime;
  RingBuffer<PathPoint> coordBuffer;
};

}  // namespace rover
