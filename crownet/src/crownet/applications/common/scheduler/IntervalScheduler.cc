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
#include "crownet/crownet.h"

using namespace inet;

using namespace crownet::queueing;

namespace crownet {

Define_Module(IntervalScheduler)


void IntervalScheduler::initialize(int stage)
{
    AppSchedulerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        numberPackets = &par("numberPackets");
        amoutOfData = &par("amoutOfData");

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
    simtime_t now  = simTime();
    auto delay = generationIntervalParameter->doubleValue();
    if (delay < 0){
        EV_INFO << LOG_MOD << " generationIntervalParameter < 0. Deactivate AppScheduler" << endl;
        stopScheduling = true;
    } else if ((app->getStopTime() > simtime_t::ZERO) && (simTime() > app->getStopTime())) {
        EV_INFO << LOG_MOD << " App stop time reached. Do not schedule any more data" << endl;
        stopScheduling = true;
        // todo emit signal
    } else {
        auto scheduleAt = std::max(now, app->getStartTime()) + delay;
        EV_INFO << simTime().ustr() << " schedule application" << endl;
        scheduleClockEventAt(scheduleAt, generationTimer);
    }
}

void IntervalScheduler::handleMessage(cMessage *message)
{
    if (message == generationTimer) {
        if ((app->getStopTime() > simtime_t::ZERO) && (simTime() > app->getStopTime())){
            EV_INFO << LOG_MOD << " App stop time reached. Do not schedule any more data" << endl;
            // todo emit signal
        } else {
            scheduleApp(message);
            if (stopScheduling){
              EV << LOG_MOD << "Scheduler limit reached" << endl;
            } else {
                scheduleGenerationTimer();
            }
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
            stopScheduling = true;
        } else {
            sentPackets += numPacket;
            consumer->producePackets(numPacket);
        }

    } else if (data > b(0)){
        if(maxData > b(0) && (sentData + data) > maxData ){
            stopScheduling = true;
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
