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

#include "VruUdp.h"
#include "crownet/applications/common/AppCommon_m.h"

namespace crownet {

Define_Module(VruUdp);

VruUdp::VruUdp() {}

VruUdp::~VruUdp() {}

void VruUdp::initialize(int stage) {
  UdpBaseApp::initialize(stage);
  if (stage == INITSTAGE_APPLICATION_LAYER) {
    mobilityModule = check_and_cast<MobilityBase*>(
        getParentModule()->getSubmodule("mobility"));
  }
}

BaseApp::FsmState VruUdp::fsmAppMain(cMessage* msg) {
  const auto& payload = makeShared<ApplicationPacket>();
  payload->setSequenceNumber(numSent);
  payload->setChunkLength(B(par("messageLength")));
  payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
  payload->addTag<CurrentLocationTag>()->setLocation(getCurrentLocation());
  sendPayload(payload);
  scheduleNextAppMainEvent();
  return FsmRootStates::WAIT_ACTIVE;
}

Coord VruUdp::getCurrentLocation() {
  return mobilityModule->getCurrentPosition();
}
}  // namespace crownet
