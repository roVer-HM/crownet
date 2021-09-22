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

#include "BaseBroadcast.h"

#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

namespace crownet {

Define_Module(BaseBroadcast);


BaseBroadcast::BaseBroadcast() {}

BaseBroadcast::~BaseBroadcast() {}


void BaseBroadcast::initialize(int stage){
    BaseApp::initialize(stage);
    if (stage == INITSTAGE_LOCAL){
        initialHopCount = par("hopCount").intValue();
    }
}

Packet *BaseBroadcast::createPacket() {

    // use configured packet size
    auto payload = createPayload<ApplicationPacket>();
    payload->addTagIfAbsent<HopCount>()->setHops(initialHopCount);

    return buildPacket(payload);
}


FsmState BaseBroadcast::handlePayload(const Ptr<const ApplicationPacket> pkt){
    // no logic in BaseBroadcast
    return FsmRootStates::WAIT_ACTIVE;
}

FsmState BaseBroadcast::handleDataArrived(Packet *packet){
    auto payload = packet->popAtFront<ApplicationPacket>();
    auto addressInd  = packet->getTag<L3AddressInd>();
    auto portInd =  packet->getTag<L4PortInd>();
    if(payload->findTag<HopCount>()){
        int hops = payload->getTag<HopCount>()->getHops() -1;
        if (hops > 0){
            auto newPayload = Ptr<ApplicationPacket>(payload->dup());
            newPayload->removeTag<HopCount>(b(0), b(-1));
            auto tag = newPayload->addTagIfAbsent<HopCount>();
            tag->setHops(hops);
            scheduler->schedulePacket(buildPacket(newPayload));
        }
    }
    return handlePayload(payload);
}

}


