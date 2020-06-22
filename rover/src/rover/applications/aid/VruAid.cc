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

#include "VruAid.h"
#include "rover/aid/AidCommand_m.h"
#include "rover/applications/common/AppCommon_m.h"

namespace rover {

Define_Module(VruAid);

VruAid::VruAid() {
  // TODO Auto-generated constructor stub
}

VruAid::~VruAid() {
  // TODO Auto-generated destructor stub
}

void VruAid::initialize(int stage) {
  AidBaseApp::initialize(stage);
  if (stage == INITSTAGE_APPLICATION_LAYER) {
    mobilityModule = check_and_cast<MobilityBase*>(
        getParentModule()->getSubmodule("mobility"));
  }
}

BaseApp::FsmState VruAid::fsmSend(cMessage* msg) {
  const auto& payload = makeShared<ApplicationPacket>();
  payload->setSequenceNumber(numSent);
  payload->setChunkLength(B(par("messageLength")));
  payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
  payload->addTag<CurrentLocationTag>()->setLocation(getCurrentLocation());
  sendPayload(payload);
  scheduleNextSendEvent();
  return FsmRootStates::WAIT_ACTIVE;
}

Coord VruAid::getCurrentLocation() {
  return mobilityModule->getCurrentPosition();
}

void VruAid::setAppRequirements() {
  L3Address destAddr = chooseDestAddr();
  socket.setAppRequirements(par("minRate").doubleValue(),
                            par("maxRate").doubleValue(),
                            AidRecipientClass::ALL_LOCAL, destAddr, destPort);
}

void VruAid::setAppCapabilities() {
  // todo: no CAP right now.
}

}  // namespace rover
