/*
 * MobilityRoverArtery.h
 *
 *  Created on: Aug 10, 2020
 *      Author: sts
 */

#pragma once

#include <artery/traci/MobilityBase.h>
#include <omnetpp/csimplemodule.h>

#include "inet/mobility/contract/IMobility.h"
#include "rover/common/util/rover_util.h"
#include "rover/mobility/IPositionHistoryProvider.h"
#include "rover/rover.h"

namespace inet {
class CanvasProjection;
}

using namespace artery;
using namespace omnetpp;

namespace rover {

class MobilityRoverArtery : public artery::MobilityBase,
                            public omnetpp::cSimpleModule,
                            public inet::IMobility,
                            public IPositionHistoryProvider {
 public:
  virtual ~MobilityRoverArtery() = default;

  // artery::MobilityBase
  void initializeSink(traci::LiteAPI*, const std::string& id,
                      const traci::Boundary&,
                      std::shared_ptr<traci::VariableCache> cache) override;

  // inet::IMobility interface
  virtual int getId() const override { return cSimpleModule::getId(); }
  virtual double getMaxSpeed() const override;
  virtual const inet::Coord& getCurrentPosition() override;
  virtual const inet::Coord& getCurrentVelocity() override;
  virtual const inet::Coord& getCurrentAcceleration() override;
  virtual const inet::Quaternion& getCurrentAngularPosition() override;
  virtual const inet::Quaternion& getCurrentAngularVelocity() override;
  virtual const inet::Quaternion& getCurrentAngularAcceleration() override;
  virtual const inet::Coord& getConstraintAreaMax() const override;
  virtual const inet::Coord& getConstraintAreaMin() const override;

  // omnetpp::cSimpleModule
  void initialize(int stage) override;
  int numInitStages() const override;
  //  void setInitialPosition() override;

  virtual void move();
  virtual void moveAndUpdate();

  virtual void recoredTimeCoord(simtime_t time, inet::Coord coord) override;

  virtual std::vector<PathPoint> getPositionHistory() override;
  virtual std::vector<PathPoint> getDeltaPositionHistory() override;
  virtual int historySize() override;

 protected:
  //  virtual void handleSelfMessage(omnetpp::cMessage* message) override;
  void refreshDisplay() const override;

  virtual void emitMobilityStateChangedSignal();

  simtime_t lastUpdate;
  inet::Coord lastPosition;
  inet::Coord lastVelocity;
  inet::Quaternion lastOrientation;
  inet::Coord constrainedAreaMin;
  inet::Coord constrainedAreaMax;

  simtime_t nextPosTime;
  inet::Coord nextPosition;

  simtime_t recordThreshold;
  simtime_t lastRecordedTime;
  RingBuffer<PathPoint> coordBuffer;

 private:
  void initialize(const Position& pos, Angle heading, double speed) override;
  void update(const Position& pos, Angle heading, double speed) override;
  simtime_t getUpdateTime();

  double mAntennaHeight = 1.5;
  omnetpp::cModule* mVisualRepresentation = nullptr;
  const inet::CanvasProjection* mCanvasProjection = nullptr;
};

} /* namespace rover */
