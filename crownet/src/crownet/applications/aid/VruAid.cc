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
#include "crownet/aid/AidCommand_m.h"
#include "crownet/applications/common/AppCommon_m.h"

namespace crownet {

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
    mobilityModule = check_and_cast<IPositionHistoryProvider*>(
        getParentModule()->getSubmodule("mobility"));
  }
}

BaseApp::FsmState VruAid::fsmAppMain(cMessage* msg) {
  const auto& vam = createPacket<ItsVam>(B(par("messageLength")));  // todo calc?
  vam->setGenerationDeltaTime(simTime());
  // VAM Header
  ItsPduHeader itsHeader{};
  itsHeader.setMessageId(ItsMessageId::VAM);
  itsHeader.setProtocolVersion(1);
  itsHeader.setStationId(getId());  // componentId
  vam->setItsHeader(itsHeader);
  //
  ItsVamBasicContainer basicContainer{};
  basicContainer.setRole(ItsVehicleRole::DEFAULT);
  // todo Mixup CAM/VAM
  basicContainer.setStationType(ItsStationType::PEDSTRIAN);
  basicContainer.setVruDeviceType(ItsVruDeviceType::VRU_ST);
  basicContainer.setVruProfile(ItsVruProfile::PEDESTRIAN);
  basicContainer.setVruType(ItsVruType::ADULT);
  ReferencePosition ref{};
  ref.setHeadingValue(mobilityModule->getCurrentVelocity().getNormalized());
  ref.setReferencePosition(mobilityModule->getCurrentPosition());
  ref.setSemiMajorConf(0.3);
  ref.setSemiMajorConf(0.3);
  basicContainer.setReferencePosition(ref);
  int k = 0;
  for (const auto& p : mobilityModule->getDeltaPositionHistory()) {
    basicContainer.setPathHistory(k, p);
  }
  vam->setBasicContainer(basicContainer);

  sendPayload(vam);
  scheduleNextAppMainEvent();
  return FsmRootStates::WAIT_ACTIVE;
}

BaseApp::FsmState VruAid::handleSocketDataArrived(Packet *packet){
    // todo: implement handle packet
    // ...
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

}  // namespace crownet
