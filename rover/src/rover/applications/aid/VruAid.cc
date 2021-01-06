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
    mobilityModule = check_and_cast<IPositionHistoryProvider*>(
        getParentModule()->getSubmodule("mobility"));
  }
}

BaseApp::FsmState VruAid::fsmAppMain(cMessage* msg) {
  //  vanetza::asn1::Cam message;

  //  const auto& vam = makeShared<ItsVam>();
  //  vam->setSequenceNumber(numSent);
  //  vam->addTag<CreationTimeTag>()->setCreationTime(simTime());

  const auto& vam = createPacket<ItsVam>();
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

  vam->setChunkLength(B(par("messageLength")));  // todo calc?
  sendPayload(vam);
  scheduleNextAppMainEvent();
  return FsmRootStates::WAIT_ACTIVE;
}

void VruAid::socketDataArrived(AidSocket* socket, Packet* packet) {
  auto payload = checkEmitGetReceived<ItsVam>(packet);
  //  emit(packetReceivedSignal, packet);
  //  numReceived++;
  // todo log received coordiantes.
  delete packet;
  socketFsmResult =
      FsmRootStates::WAIT_ACTIVE;  // GoTo WAIT_ACTIVE steady state
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
