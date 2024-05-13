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

#include "crownet/applications/control/BroadcastControlApp.h"
#include "crownet/applications/common/AppCommon_m.h"
#include "crownet/applications/control/control_m.h"
#include "crownet/artery/traci/VaderePersonController.h"
#include "artery/traci/ControllablePerson.h"
#include "artery/traci/PersonMobility.h"
#include "inet/common/packet/Message.h"


namespace crownet {

Define_Module(BroadcastControlApp);

BroadcastControlApp::BroadcastControlApp() {}

BroadcastControlApp::~BroadcastControlApp() {}

void BroadcastControlApp::initialize(int stage) {
    BaseBroadcast::initialize(stage);
    if (stage == INITSTAGE_APPLICATION_LAYER) {
        if (par("isControlled").boolValue()){
            auto mobility = castMobility<ControllablePerson>();
            auto persCon = mobility->getPersonController();

            ctrl = dynamic_cast<VaderePersonController*>(persCon);
            ASSERT(ctrl);
        }

    }
}


FsmState BroadcastControlApp::handlePayload(const Ptr<const ApplicationPacket> pkt){
    // Possible rebroadcast handled in BaseBroadcast::handleDataArrived


    if (isControlled()){
            EV_INFO << getFullPath() << "send packet";
            auto ctrlTag = pkt->getTag<SimpleControlCfg>();
           // send received information over traci to vadere
            ctrl->send_control(ctrlTag->getModelString(), ctrlTag->getModelData());
    }
    return FsmRootStates::WAIT_ACTIVE;
}

Packet *BroadcastControlApp::createPacket() {

    // use configured packet size.
    auto payload = createPayload<ApplicationPacket>();
    payload->addTagIfAbsent<HopCount>()->setHops(initialHopCount);

    auto meta = payload->addTagIfAbsent<SimpleControlCfg>();
    meta->setModelString(modelString.c_str());
    meta->setModelData(modelData.c_str());

    return buildPacket(payload);
}

FsmState BroadcastControlApp::fsmHandleSubState(cMessage *msg){
    // called by scheduler using the BaseApp FSM. Just create and send control message

    auto cfgMsg = check_and_cast<Message*>(msg);
    auto controlCfg = cfgMsg->getTag<SimpleControlCfg>();

    // save last received data to possible retransmissions.
    modelString = controlCfg->getModelString();
    modelData = controlCfg->getModelData();
    modelAge = simTime();

    auto packet = createPacket();

    pushOrSendPacket(packet, outputGate, consumer);

    return FsmRootStates::WAIT_ACTIVE;
}

}
