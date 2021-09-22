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

#include "crownet/applications/common/scheduler/IntervalScheduler.h"

#include "inet/common/ModuleAccess.h"

using namespace inet;

using namespace crownet::queueing;

namespace crownet {

Define_Module(IntervalScheduler)


void IntervalScheduler::initialize(int stage)
{
    AppSchedulerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        numberPackets = &par("numberPackets");
        amoutOfData = &par("maxData");

        maxNumberPackets = par("maxNumberPackets").intValue();
        maxData = b(par("maxData").intValue());

        generationIntervalParameter = &par("generationInterval");
        generationTimer = new ClockEvent("GenerationTimer");
    }
    else if (stage == INITSTAGE_QUEUEING)
        scheduleGenerationTimer();
}

void IntervalScheduler::scheduleGenerationTimer()
{
    auto delay = generationIntervalParameter->doubleValue();
    if (delay < 0){
        EV_INFO << "generationIntervalParameter < 0. Deactivate AppScheduler" << endl;
    } else {
        scheduleClockEventAfter(delay, generationTimer);
    }
}

void IntervalScheduler::handleMessage(cMessage *message)
{
    if (message == generationTimer) {
        scheduleApp(message);
        if (stop){
          EV << "Limit reached" << endl;
        } else {
            scheduleGenerationTimer();
        }
        updateDisplayString();
    } else{
       throw cRuntimeError("Unknown message");
    }
}

void IntervalScheduler::scheduleApp(cMessage *message){
    Enter_Method("scheduleApp");
    auto numPacket = numberPackets->intValue();
    auto data = b(amoutOfData->intValue());
    if (numPacket > 0){

        if(maxNumberPackets > 0 && (sentPackets + numPacket) > maxNumberPackets){
            stop = true;
        } else {
            sentPackets += numPacket;
            consumer->producePackets(numPacket);
        }

    } else if (data > b(0)){
        if(maxData > b(0) && (sentData + data) > maxData ){
            stop = true;
        } else {
            sentData += data;
            //convert to Bytes
            consumer->producePackets(B(data));
        }
    } else {
        throw cRuntimeError("Either number of packer or max data in Byte must be set");
    }
}


}
