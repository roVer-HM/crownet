/*
 * InetVaderePersonMobility.cc
 *
 *  Created on: Aug 10, 2020
 *      Author: sts
 */

#include "InetVaderePersonMobility.h"

#include <inet/common/ModuleAccess.h>
#include <inet/common/geometry/common/CanvasProjection.h>
#include <inet/common/geometry/common/Coord.h>
#include <inet/features.h>
#include <omnetpp/csimplemodule.h>
#include <cmath>
#include "crownet/artery/traci/VaderePersonController.h"
#include "crownet/artery/traci/VariableCache.h"
#include "crownet/artery/traci/VadereApi.h"

#ifdef WITH_VISUALIZERS
#include <inet/visualizer/mobility/MobilityCanvasVisualizer.h>
#else
#include <cstdio>
#endif

namespace crownet {

Define_Module(InetVaderePersonMobility)

    int InetVaderePersonMobility::numInitStages() const {
  return inet::INITSTAGE_PHYSICAL_ENVIRONMENT + 1;
}

void InetVaderePersonMobility::initialize(int stage) {
  //  inet::MobilityBase::initialize(stage);
  if (stage == inet::INITSTAGE_LOCAL) {
    mVisualRepresentation = inet::findModuleFromPar<cModule>(
        par("visualRepresentation"), this);
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

void InetVaderePersonMobility::initializeSink(std::shared_ptr<API> api, std::shared_ptr<VaderePersonCache> cache, const Boundary& boundary) {
  ASSERT(api);
  ASSERT(cache);
  mTraci = api;
  mNetBoundary = boundary;
  mCache = cache;
  mObjectId = cache->getId(); //FIXME mPersonId


  const auto& max = mNetBoundary.upperRightPosition();
  constrainedAreaMax = inet::Coord { max.x, max.y, max.z };

  const auto& min = mNetBoundary.lowerLeftPosition();
  constrainedAreaMin = inet::Coord { min.x, min.y, min.z };

  std::shared_ptr<crownet::VaderePersonCache> vehicleCache =
      std::dynamic_pointer_cast<crownet::VaderePersonCache>(cache);

  if (!vehicleCache) {
    // todo
  }
  mController.reset(new VaderePersonController(vehicleCache));
}


void InetVaderePersonMobility::initializePerson(const TraCIPosition& traci_pos, TraCIAngle traci_heading, double traci_speed)
{
    auto position = position_cast(mNetBoundary, traci_pos);
    auto heading = angle_cast(traci_heading);
    auto speed = traci_speed;
    initialize(position, heading, speed);
}

void InetVaderePersonMobility::updatePerson(const TraCIPosition& traci_pos, TraCIAngle traci_heading, double traci_speed)
{
    if (usesEmulatedPosition) return;

    auto position = position_cast(mNetBoundary, traci_pos);
    auto heading = angle_cast(traci_heading);
    auto speed = traci_speed;

    update(position, heading, speed);
}



double InetVaderePersonMobility::getMaxSpeed() const { return NaN; }

const inet::Coord& InetVaderePersonMobility::getCurrentPosition() {
  moveAndUpdate();
  return lastPosition;
}

const inet::Coord& InetVaderePersonMobility::getCurrentVelocity() {
  moveAndUpdate();
  return lastVelocity;
}

const inet::Coord& InetVaderePersonMobility::getCurrentAcceleration() {
  //  return inet::Coord::NIL;
  throw cRuntimeError("Invalid operation");
}

const inet::Quaternion& InetVaderePersonMobility::getCurrentAngularPosition() {
  moveAndUpdate();
  return lastOrientation;
}

const inet::Quaternion& InetVaderePersonMobility::getCurrentAngularVelocity() {
  //  return inet::Quaternion::NIL;
  throw cRuntimeError("Invalid operation");
}

const inet::Quaternion& InetVaderePersonMobility::getCurrentAngularAcceleration() {
  //  return inet::Quaternion::NIL;
  throw cRuntimeError("Invalid operation");
}

const inet::Coord& InetVaderePersonMobility::getConstraintAreaMax() const {
    return constrainedAreaMax;
}

const inet::Coord& InetVaderePersonMobility::getConstraintAreaMin() const {
    return constrainedAreaMin;
}

void InetVaderePersonMobility::initialize(const Position& pos, Angle heading,
                                     double speed) {
  // setup coordBuffer. Must be done here and not in initialize due to dynamic
  // creation with TraCI.
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

void InetVaderePersonMobility::update(const Position& pos, Angle heading,
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

omnetpp::simtime_t InetVaderePersonMobility::getUpdateTime() {

  auto vApi = std::dynamic_pointer_cast<VadereApi>(mTraci);
  return vApi->v_simulation.getTime();
}

void InetVaderePersonMobility::emitMobilityStateChangedSignal() {
  ASSERT(inet::IMobility::mobilityStateChangedSignal ==
         artery::MobilityBase::stateChangedSignal);
  emit(artery::MobilityBase::stateChangedSignal, this);
}

void InetVaderePersonMobility::refreshDisplay() const {
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

void InetVaderePersonMobility::moveAndUpdate() {
  simtime_t now = simTime();
  if (lastUpdate != now && !usesEmulatedPosition) {
    move();
    lastUpdate = simTime();
    emitMobilityStateChangedSignal();
  }
  recoredTimeCoord(lastUpdate, lastPosition);
}

void InetVaderePersonMobility::move() {
  double elapsedTime = (simTime() - lastUpdate).dbl();
  lastPosition += lastVelocity * elapsedTime;
}

std::vector<PathPoint> InetVaderePersonMobility::getPositionHistory() {
  std::vector<PathPoint> ret = coordBuffer.getData(true);  // new->old
  return ret;
}

std::vector<PathPoint> InetVaderePersonMobility::getDeltaPositionHistory() {
  PathPoint basePoint(lastPosition, lastUpdate);
  std::vector<PathPoint> history = getPositionHistory();
  for (int i = 0; i < history.size(); i++) {
    history[i] = basePoint - history[i];
  }
  return history;
}

void InetVaderePersonMobility::recoredTimeCoord(const simtime_t& time, const inet::Coord& coord) {
  if ((time - lastRecordedTime) > recordThreshold) {
    coordBuffer.put(PathPoint(coord, time));
    lastRecordedTime = time;
  }
  if (time > 1.0) {
    getPositionHistory();
  }
}

int InetVaderePersonMobility::historySize() { return coordBuffer.size(); }

void InetVaderePersonMobility::useEmulatedPositionSource(bool use) {
    usesEmulatedPosition = use;
}

bool InetVaderePersonMobility::usesEmulatedPositionSource() {
    return usesEmulatedPosition;
}

void InetVaderePersonMobility::injectEmulatedPosition(inet::Coord pos, Angle heading, double speed) {
    auto position = Position{pos.getX(), pos.getY()};
    lastUpdate = 0.0;
    update(position, heading, speed);
}

} /* namespace crownet */
