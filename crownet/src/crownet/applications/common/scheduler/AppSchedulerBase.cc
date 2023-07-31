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

#include "AppSchedulerBase.h"

#include "inet/common/ModuleAccess.h"

using namespace inet;

using namespace crownet::queueing;

namespace crownet {

void AppSchedulerBase::receiveSignal(cComponent *source, simsignal_t signalID, intval_t i, cObject *details) {
    if (signalID == servingCell_){
        eNBId = i; // todo add some delay until application knows about the loss of connection?
    }
}


void AppSchedulerBase::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        outputGate = gate("out");
        schedulerIn = gate("schedulerIn");
        servingCell_ = registerSignal("servingCell");
        getContainingNode(this)->subscribe(servingCell_, this);
        checkNetworkConnectivity = par("checkNetworkConnectivity");
        consumer = findConnectedModule<ICrownetActivePacketSource>(outputGate);
        app = findConnectedModule<AppStatusInfo>(outputGate);
    }
}

void AppSchedulerBase::assertAppRunning() const{
    if (app->isStopped()){
        throw cRuntimeError("Connected app is not running");
    }
}

void AppSchedulerBase::handleMessage(cMessage *message)
{
    if (message->isSelfMessage()) {
        scheduleApp(message);
    } else if (message->arrivedOn(schedulerIn->getId())){
        if (message->isPacket()){
            schedulePacket(check_and_cast<Packet*>(message));
        } else {
            scheduleEvent(message);
        }
    } else{
       throw cRuntimeError("Unknown message");
    }
    updateDisplayString();
}

}



