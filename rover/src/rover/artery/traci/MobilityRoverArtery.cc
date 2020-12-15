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
    WATCH(lastPosition);
    WATCH(lastVelocity);
    WATCH(lastOrientation);
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

inet::Coord MobilityRoverArtery::getCurrentPosition() {
  moveAndUpdate();
  return lastPosition;
}

inet::Coord MobilityRoverArtery::getCurrentVelocity() {
  moveAndUpdate();
  return lastVelocity;
}

inet::Coord MobilityRoverArtery::getCurrentAcceleration() {
  //  return inet::Coord::NIL;
  throw cRuntimeError("Invalid operation");
}

inet::Quaternion MobilityRoverArtery::getCurrentAngularPosition() {
  moveAndUpdate();
  return lastOrientation;
}

inet::Quaternion MobilityRoverArtery::getCurrentAngularVelocity() {
  //  return inet::Quaternion::NIL;
  throw cRuntimeError("Invalid operation");
}

inet::Quaternion MobilityRoverArtery::getCurrentAngularAcceleration() {
  //  return inet::Quaternion::NIL;
  throw cRuntimeError("Invalid operation");
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
  // setup coordBuffer. Must be done here and not in initialize due to dynamic
  // creation with TraCI/veins.
  coordBuffer.set(par("ringBufferSize").intValue());
  recordThreshold = par("recordThreshold").doubleValue();
  lastRecordedTime = SIMTIME_ZERO;

  using boost::units::si::meter;
  const double heading_rad = heading.radian();
  const inet::Coord direction{cos(heading_rad), -sin(heading_rad)};

  nextPosTime = getUpdateTime();
  nextPosition = inet::Coord{pos.x / meter, pos.y / meter, mAntennaHeight};

  lastUpdate = simTime();
  lastPosition = nextPosition;
  lastVelocity = direction * speed;
  inet::EulerAngles angles;
  angles.alpha = inet::units::values::rad(-heading_rad);
  lastOrientation = inet::Quaternion(angles);

  // ensure that first PathPoint is recoreded.
  coordBuffer.put(PathPoint(lastPosition, lastUpdate));
  lastRecordedTime = lastUpdate;

  //  mPosition = inet::Coord{pos.x / meter, pos.y / meter, mAntennaHeight};
  //  mVelocity = direction * speed;
  //  inet::EulerAngles angles;
  //  angles.alpha = inet::units::values::rad(-heading_rad);
  //  mOrientation = inet::Quaternion(angles);
}

void MobilityRoverArtery::update(const Position& pos, Angle heading,
                                 double speed) {
  if (lastUpdate == simTime()) return;  // do not update twice
  using boost::units::si::meter;
  const double heading_rad = heading.radian();
  const inet::Coord direction{cos(heading_rad), -sin(heading_rad)};

  lastPosition = nextPosition;
  lastUpdate = nextPosTime;

  nextPosition = inet::Coord{pos.x / meter, pos.y / meter, mAntennaHeight};
  nextPosTime = getUpdateTime();
  lastVelocity =
      (nextPosition - lastPosition) / (nextPosTime - lastUpdate).dbl();

  inet::EulerAngles angles;
  angles.alpha = inet::units::values::rad(-heading_rad);
  lastOrientation = inet::Quaternion(angles);

  recoredTimeCoord(lastUpdate, lastPosition);

  //  initialize(pos, heading, speed);
  emitMobilityStateChangedSignal();
}

omnetpp::simtime_t MobilityRoverArtery::getUpdateTime() {
  auto vApi = omnetpp::check_and_cast<VadereLiteApi*>(mTraci);
  return vApi->vSimulation().getTime();
}

void MobilityRoverArtery::emitMobilityStateChangedSignal() {
  ASSERT(inet::IMobility::mobilityStateChangedSignal ==
         artery::MobilityBase::stateChangedSignal);
  emit(artery::MobilityBase::stateChangedSignal, this);
}

void MobilityRoverArtery::refreshDisplay() const {
  // following code is taken from INET's MobilityBase::refreshDisplay
  if (mVisualRepresentation) {
    //    moveAndUpdate();
    auto position = mCanvasProjection->computeCanvasPoint(lastPosition);
    char buf[32];
    snprintf(buf, sizeof(buf), "%lf", position.x);
    buf[sizeof(buf) - 1] = 0;
    mVisualRepresentation->getDisplayString().setTagArg("p", 0, buf);
    snprintf(buf, sizeof(buf), "%lf", position.y);
    buf[sizeof(buf) - 1] = 0;
    mVisualRepresentation->getDisplayString().setTagArg("p", 1, buf);
  }
}

void MobilityRoverArtery::moveAndUpdate() {
  simtime_t now = simTime();
  if (lastUpdate != now) {
    move();
    lastUpdate = simTime();
    emitMobilityStateChangedSignal();
  }
  recoredTimeCoord(lastUpdate, lastPosition);
}

void MobilityRoverArtery::move() {
  double elapsedTime = (simTime() - lastUpdate).dbl();
  lastPosition += lastVelocity * elapsedTime;
}

std::vector<PathPoint> MobilityRoverArtery::getPositionHistory() {
  std::vector<PathPoint> ret = coordBuffer.getData(true);  // new->old
  return ret;
}

std::vector<PathPoint> MobilityRoverArtery::getDeltaPositionHistory() {
  PathPoint basePoint(lastPosition, lastUpdate);
  std::vector<PathPoint> history = getPositionHistory();
  for (int i = 0; i < history.size(); i++) {
    history[i] = basePoint - history[i];
  }
  return history;
}

void MobilityRoverArtery::recoredTimeCoord(simtime_t time, inet::Coord coord) {
  if ((time - lastRecordedTime) > recordThreshold) {
    coordBuffer.put(PathPoint(coord, time));
    lastRecordedTime = time;
  }
  if (time > 1.0) {
    getPositionHistory();
  }
}

int MobilityRoverArtery::historySize() { return coordBuffer.size(); }

} /* namespace rover */
