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

#include "VruEtsiSimple.h"

#include "crownet/aid/AidCommand_m.h"
#include "crownet/applications/common/AppCommon_m.h"
#include "inet/common/ModuleAccess.h"

namespace crownet {

Define_Module(VruEtsiSimple);

VruEtsiSimple::VruEtsiSimple() {
}

VruEtsiSimple::~VruEtsiSimple() {
}

void VruEtsiSimple::initialize(int stage) {
  BaseApp::initialize(stage);
  if (stage == INITSTAGE_APPLICATION_LAYER) {
    mobilityModule = inet::getModuleFromPar<IPositionHistoryProvider>(par("mobilityModule"), this);
  }
}

//CrownetPacketSourceBase
Packet *VruEtsiSimple::createPacket(){
    const auto& vam = createPayload<ItsVam>(B(par("messageLength")));  // todo calc?
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

    return buildPacket(vam);
}

FsmState VruEtsiSimple::handleDataArrived(Packet *packet){
    // todo: implement handle packet
    // ...

    return FsmRootStates::WAIT_ACTIVE;
}

Coord VruEtsiSimple::getCurrentLocation() {
  return mobilityModule->getCurrentPosition();
}


}  // namespace crownet
