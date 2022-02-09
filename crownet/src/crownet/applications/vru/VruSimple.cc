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

#include "VruSimple.h"

#include "crownet/applications/common/AppCommon_m.h"

namespace crownet {

Define_Module(VruSimple);

VruSimple::VruSimple() {}

VruSimple::~VruSimple() {}

void VruSimple::initialize(int stage) {
  BaseApp::initialize(stage);
  if (stage == INITSTAGE_APPLICATION_LAYER) {
    mobilityModule = castMobility<MobilityBase>();
  }
}

//CrownetPacketSourceBase
Packet *VruSimple::createPacket(){
    const auto& payload = makeShared<ApplicationPacket>();
    payload->setSequenceNumber(localInfo->nextSequenceNumber());
    payload->setChunkLength(b(par("packetLength").intValue()));
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    payload->addTag<CurrentLocationTag>()->setLocation(getCurrentLocation());


    return buildPacket(payload);
}

FsmState VruSimple::handleDataArrived(Packet *packet){
    return FsmRootStates::WAIT_ACTIVE;
}

Coord VruSimple::getCurrentLocation() {
  return mobilityModule->getCurrentPosition();
}
}  // namespace crownet
