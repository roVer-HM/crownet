/*
 * MobilityRoverArtery.cc
 *
 *  Created on: Aug 10, 2020
 *      Author: sts
 */

#include "MobilityRoverArtery.h"
#include <inet/common/ModuleAccess.h>
#include <inet/common/geometry/common/CanvasProjection.h>
#include <inet/common/geometry/common/Coord.h>
#include <inet/features.h>
#include <omnetpp/csimplemodule.h>
#include <cmath>
#include "rover/artery/traci/VaderePersonController.h"
#include "rover/artery/traci/VariableCache.h"

#ifdef WITH_VISUALIZERS
#include <inet/visualizer/mobility/MobilityCanvasVisualizer.h>
#else
#include <cstdio>
#endif

namespace rover {

Define_Module(MobilityRoverArtery)

    int MobilityRoverArtery::numInitStages() const {
  return inet::INITSTAGE_PHYSICAL_ENVIRONMENT + 1;
}

void MobilityRoverArtery::initialize(int stage) {
  //  inet::MobilityBase::initialize(stage);
  if (stage == inet::INITSTAGE_LOCAL) {
    mVisualRepresentation = inet::getModuleFromPar<cModule>(
        par("visualRepresentation"), this, false);
    WATCH(mObjectId);
    WATCH(mPosition);
    WATCH(mSpeed);
    WATCH(mOrientation);
  } else if (stage == inet::INITSTAGE_PHYSICAL_ENVIRONMENT) {
    if (mVisualRepresentation) {
      auto visualizationTarget = mVisualRepresentation->getParentModule();
      mCanvasProjection = inet::CanvasProjection::getCanvasProjection(
          visualizationTarget->getCanvas());
    }
    emit(artery::MobilityBase::stateChangedSignal, this);
  }
}

void MobilityRoverArtery::initializeSink(
    traci::LiteAPI* api, const std::string& id, const traci::Boundary& boundary,
    std::shared_ptr<traci::VariableCache> cache) {
  ASSERT(api);
  ASSERT(cache);
  ASSERT(cache->getId() == id);
  ASSERT(&cache->getLiteAPI() == api);
  mTraci = api;
  mObjectId = id;
  mNetBoundary = boundary;

  std::shared_ptr<rover::VaderePersonCache> vehicleCache =
      std::dynamic_pointer_cast<rover::VaderePersonCache>(cache);
  if (!vehicleCache) {
    // todo
  }
  mController.reset(new VaderePersonController(vehicleCache));
}

double MobilityRoverArtery::getMaxSpeed() const { return NaN; }

inet::Coord MobilityRoverArtery::getCurrentPosition() { return mPosition; }

inet::Coord MobilityRoverArtery::getCurrentVelocity() { return mSpeed; }

inet::Coord MobilityRoverArtery::getCurrentAcceleration() {
  return inet::Coord::NIL;
}

inet::Quaternion MobilityRoverArtery::getCurrentAngularPosition() {
  return mOrientation;
}

inet::Quaternion MobilityRoverArtery::getCurrentAngularVelocity() {
  return inet::Quaternion::NIL;
}

inet::Quaternion MobilityRoverArtery::getCurrentAngularAcceleration() {
  return inet::Quaternion::NIL;
}

inet::Coord MobilityRoverArtery::getConstraintAreaMax() const {
  const auto& max = mNetBoundary.upperRightPosition();
  return inet::Coord{max.x, max.y, max.z};
}

inet::Coord MobilityRoverArtery::getConstraintAreaMin() const {
  const auto& min = mNetBoundary.lowerLeftPosition();
  return inet::Coord{min.x, min.y, min.z};
}

void MobilityRoverArtery::initialize(const Position& pos, Angle heading,
                                     double speed) {
  using boost::units::si::meter;
  const double heading_rad = heading.radian();
  const inet::Coord direction{cos(heading_rad), -sin(heading_rad)};
  mPosition = inet::Coord{pos.x / meter, pos.y / meter, mAntennaHeight};
  mSpeed = direction * speed;
  inet::EulerAngles angles;
  angles.alpha = inet::units::values::rad(-heading_rad);
  mOrientation = inet::Quaternion(angles);
}

void MobilityRoverArtery::update(const Position& pos, Angle heading,
                                 double speed) {
  initialize(pos, heading, speed);
  ASSERT(inet::IMobility::mobilityStateChangedSignal ==
         artery::MobilityBase::stateChangedSignal);
  emit(artery::MobilityBase::stateChangedSignal, this);
}

void MobilityRoverArtery::refreshDisplay() const {
  // following code is taken from INET's MobilityBase::refreshDisplay
  if (mVisualRepresentation) {
    auto position = mCanvasProjection->computeCanvasPoint(mPosition);
    char buf[32];
    snprintf(buf, sizeof(buf), "%lf", position.x);
    buf[sizeof(buf) - 1] = 0;
    mVisualRepresentation->getDisplayString().setTagArg("p", 0, buf);
    snprintf(buf, sizeof(buf), "%lf", position.y);
    buf[sizeof(buf) - 1] = 0;
    mVisualRepresentation->getDisplayString().setTagArg("p", 1, buf);
  }
}

} /* namespace rover */
