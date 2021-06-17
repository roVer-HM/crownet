/*
 * InetVaderePersonMobility.h
 *
 *  Created on: Aug 10, 2020
 *      Author: sts
 */

#pragma once

#include <artery/traci/MobilityBase.h>
#include <artery/inet/InetMobility.h>
#include <artery/utility/Geometry.h>
#include <omnetpp/csimplemodule.h>

#include "inet/mobility/contract/IMobility.h"
#include "crownet/common/util/crownet_util.h"
#include "crownet/mobility/IPositionHistoryProvider.h"
#include "crownet/artery/traci/VaderePersonSink.h"
#include "crownet/crownet.h"

namespace inet {
class CanvasProjection;
}

using namespace artery;
using namespace omnetpp;

namespace crownet {

//todo: check if still working after update?
class InetVaderePersonMobility : public InetMobility,
                            public VaderePersonSink,
                            public IPositionHistoryProvider {
 public:
  virtual ~InetVaderePersonMobility() = default;

  // traci::PersonSink interface
  virtual void initializeSink(std::shared_ptr<traci::API> api, std::shared_ptr<VaderePersonCache>, const traci::Boundary&) override;
  virtual void initializePerson(const TraCIPosition&, TraCIAngle, double speed) override;
  virtual void updatePerson(const TraCIPosition&, TraCIAngle, double speed) override;

  const std::string& getPersonId() const { return mObjectId; }


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


  virtual void move();
  virtual void moveAndUpdate();

  // IPositionHistoryProvider
  virtual void recoredTimeCoord(const simtime_t& time, const inet::Coord& coord) override;

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

 protected:
  void initialize(const Position& pos, Angle heading, double speed) override;
  void update(const Position& pos, Angle heading, double speed) override;
  simtime_t getUpdateTime();

  std::shared_ptr<VaderePersonCache> mCache;
  double mAntennaHeight = 1.5;
  omnetpp::cModule* mVisualRepresentation = nullptr;
  const inet::CanvasProjection* mCanvasProjection = nullptr;
};

} /* namespace crownet */
