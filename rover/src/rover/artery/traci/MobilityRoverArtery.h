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

namespace inet {
class CanvasProjection;
}

using namespace artery;

namespace rover {

class MobilityRoverArtery : public artery::MobilityBase,
                            public omnetpp::cSimpleModule,
                            public inet::IMobility {
 public:
  virtual ~MobilityRoverArtery() = default;

  // artery::MobilityBase
  void initializeSink(traci::LiteAPI*, const std::string& id,
                      const traci::Boundary&,
                      std::shared_ptr<traci::VariableCache> cache) override;

  // inet::IMobility interface
  double getMaxSpeed() const override;
  inet::Coord getCurrentPosition() override;
  inet::Coord getCurrentVelocity() override;
  inet::Coord getCurrentAcceleration() override;
  inet::Quaternion getCurrentAngularPosition() override;
  inet::Quaternion getCurrentAngularVelocity() override;
  inet::Quaternion getCurrentAngularAcceleration() override;
  inet::Coord getConstraintAreaMax() const override;
  inet::Coord getConstraintAreaMin() const override;

  // omnetpp::cSimpleModule
  void initialize(int stage) override;
  int numInitStages() const override;
  //  void setInitialPosition() override;

 protected:
  //  virtual void handleSelfMessage(omnetpp::cMessage* message) override;
  void refreshDisplay() const override;

 private:
  void initialize(const Position& pos, Angle heading, double speed) override;
  void update(const Position& pos, Angle heading, double speed) override;

  inet::Coord mPosition;
  inet::Coord mSpeed;
  inet::Quaternion mOrientation;
  double mAntennaHeight = 1.5;
  omnetpp::cModule* mVisualRepresentation = nullptr;
  const inet::CanvasProjection* mCanvasProjection = nullptr;
};

} /* namespace rover */
