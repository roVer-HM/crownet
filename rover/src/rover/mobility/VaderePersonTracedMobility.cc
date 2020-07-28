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

#include "VaderePersonTracedMobility.h"

namespace rover {

Register_Class(VaderePersonTracedMobility);

void VaderePersonTracedMobility::initialize(int stage) {
  veins::VaderePersonMobility::initialize(stage);
}

void VaderePersonTracedMobility::preInitialize(
    std::shared_ptr<IMobileAgent> mobileAgent) {
  // setup coordBuffer. Must be done here and not in initialize due to dynamic
  // creation with TraCI/veins.
  coordBuffer.set(par("ringBufferSize").intValue());
  recordThreshold = par("recordThreshold").doubleValue();
  lastRecordedTime = SIMTIME_ZERO;

  veins::VaderePersonMobility::preInitialize(mobileAgent);

  // ensure that first PathPoint is recoreded.
  coordBuffer.put(PathPoint(lastPosition, lastUpdate));
  lastRecordedTime = lastUpdate;
}

void VaderePersonTracedMobility::nextPosition(
    std::shared_ptr<IMobileAgent> mobileAgent) {
  veins::VaderePersonMobility::nextPosition(mobileAgent);
  recoredTimeCoord(lastUpdate, lastPosition);
}

void VaderePersonTracedMobility::moveAndUpdate() {
  veins::VaderePersonMobility::moveAndUpdate();
  recoredTimeCoord(lastUpdate, lastPosition);
}

std::vector<PathPoint> VaderePersonTracedMobility::getPositionHistory() {
  std::vector<PathPoint> ret = coordBuffer.getData(true);  // new->old
  return ret;
}

std::vector<PathPoint> VaderePersonTracedMobility::getDeltaPositionHistory() {
  PathPoint basePoint(lastPosition, lastUpdate);
  std::vector<PathPoint> history = getPositionHistory();
  for (int i = 0; i < history.size(); i++) {
    history[i] = basePoint - history[i];
  }
  return history;
}

void VaderePersonTracedMobility::recoredTimeCoord(simtime_t time,
                                                  inet::Coord coord) {
  if ((time - lastRecordedTime) > recordThreshold) {
    coordBuffer.put(PathPoint(coord, time));
    lastRecordedTime = time;
  }
  if (time > 1.0) {
    getPositionHistory();
  }
}

int VaderePersonTracedMobility::historySize() { return coordBuffer.size(); }

}  // namespace rover
