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
#include <artery/traci/ControllableObject.h>
#include "inet/common/packet/Message.h"


namespace crownet {

Define_Module(BroadcastControlApp);

BroadcastControlApp::BroadcastControlApp() {
    // TODO Auto-generated constructor stub

}

BroadcastControlApp::~BroadcastControlApp() {
    // TODO Auto-generated destructor stub
}

void BroadcastControlApp::initialize(int stage) {
    BaseBroadcast::initialize(stage);
    if (stage == INITSTAGE_APPLICATION_LAYER) {
        if (par("isControlled").boolValue()){
            auto mobility = inet::getModuleFromPar<ControllableObject>(
                par("mobilityModule"), inet::getContainingNode(this));
            ctrl = mobility->getController<VaderePersonController>();
        }

    }
}


FsmState BroadcastControlApp::handlePayload(const Ptr<const ApplicationPacket> pkt){
    if (isControlled()){
            auto ctrlTag = pkt->getTag<SimpleControlCfg>();
            ctrl->send_control(ctrlTag->getModelString(), ctrlTag->getModelData());
    }
    return FsmRootStates::WAIT_ACTIVE;
}


FsmState BroadcastControlApp::fsmAppMain(cMessage *msg){
    auto payload = createPacket<ApplicationPacket>(B(par("messageLength")));

    payload->addTagIfAbsent<HopCount>()->setHops(initialHopCount);
    auto meta = payload->addTagIfAbsent<SimpleControlCfg>();
    meta->setModelString(modelString.c_str());
    meta->setModelData(modelData.c_str());

    sendPayload(payload);
    scheduleNextAppMainEvent();
    return FsmRootStates::WAIT_ACTIVE;
}

void BroadcastControlApp::setupTimers(){
    // do not start main loop. This will be done over HandleSubState
}



FsmState BroadcastControlApp::fsmHandleSubState(cMessage *msg){
    auto cfgMsg = check_and_cast<Message*>(msg);
    auto controlCfg = cfgMsg->getTag<SimpleControlCfg>();

    modelString = controlCfg->getModelString();
    modelData = controlCfg->getModelData();

    // reset sendLimit
    sendLimit = par("mainMsgLimit").intValue();

    // cancel main app loop and reset with received  configuration.
    cancelAppMainEvent();
    scheduleNextAppMainEvent(simTime() + appMainIntervalOffset);

    return FsmRootStates::WAIT_ACTIVE;
}

}
