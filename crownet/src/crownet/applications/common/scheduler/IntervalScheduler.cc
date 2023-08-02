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

using namespace inet;

using namespace crownet::queueing;

namespace crownet {

Define_Module(IntervalScheduler)

simsignal_t IntervalScheduler::scheduledData_s = cComponent::registerSignal("scheduledData_s");
simsignal_t IntervalScheduler::consumedData_s = cComponent::registerSignal("consumedData_s");


void IntervalScheduler::initialize(int stage)
{
    AppSchedulerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        numberPackets = &par("numberPackets");
        amountOfData = &par("amountOfData");

        maxNumberPackets = par("maxNumberPackets").intValue();
        maxData = b(par("maxData").intValue());
        startOffset = par("startOffset");

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
        clocktime_t scheduleAt = SIMTIME_AS_CLOCKTIME(std::max(now, app->getStartTime()) + delay);
        if (startOffset > simtime_t::ZERO){
            scheduleAt += SIMTIME_AS_CLOCKTIME(startOffset);
            startOffset = 0.0;
        }
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
              EV_INFO << LOG_MOD << "Scheduler limit reached" << endl;
            } else {
                scheduleGenerationTimer();
            }
        }
        updateDisplayString();
    } else{
       throw cRuntimeError("Unknown message");
    }
}

const bool IntervalScheduler::packetMaximumReached(const int& scheduledPacketCount) const {
    return hasPacketMaximum() && (sentPackets + scheduledPacketCount) > maxNumberPackets;
}

const bool IntervalScheduler::dataMaximumReached(const b& scheduledData) const{
    return hasDataMaximum() && (sentData + scheduledData) > maxData;
}

IntervalScheduler::Unit IntervalScheduler::getScheduledUnit(){
    auto numPacket = numberPackets->intValue();
    auto data = b(amountOfData->intValue());

    IntervalScheduler::Unit unit;
    unit.type = IntervalSchedulerType::OTHER;
    unit.packetCount = numPacket;
    unit.data = data;

    if (numPacket > 0) {
        unit.type = IntervalSchedulerType::PACKET;
    } else if (data > b(0)) {
        unit.type = IntervalSchedulerType::DATA;
    } else {
        throw cRuntimeError("expected numberPackets '%ld' or  amountOfData '%s' to be postive.",
                numPacket, data.str().c_str());
    }
    return unit;
}

void IntervalScheduler::scheduleByPackets(const Unit& unit){
    if (!unit.validPacketUnit()){
        throw cRuntimeError("scheduling based on number of packets with invalid packet unit '%s'.", unit.str().c_str());
    }
    // schedule packet based
    if(packetMaximumReached(unit.packetCount)){
        stopScheduling = true;
    } else {
        app->setScheduleData(b(-1)); // just produce packets regardless of size
        if (app->canProducePacket()){
            // can produce at least one packet
            EV_INFO << LOG_MOD << " schedule packet quota " << unit.packetCount << " packet(s)" << endl;
            consumer->producePackets(unit.packetCount);
            sentPackets += unit.packetCount;
        } else {
            EV_INFO << LOG_MOD << "No data in application. Scheduled data dropped" << endl;
        }
        app->setScheduleData(b(0));
    }

}

void IntervalScheduler::scheduleByData(const Unit& unit){

    if (!unit.validDataUnit()){
        throw cRuntimeError("scheduling based on amount of data with invalid data unit '%s'.", unit.str().c_str());
    }
    // schedule amount based
    if(dataMaximumReached(unit.data)){
       stopScheduling = true;
    } else {
       app->setScheduleData(unit.data);
       emit(scheduledData_s, unit.data.get());
       if (app->canProducePacket()){
           EV_INFO << LOG_MOD << " schedule data quota " << unit.data.str() << endl;
           consumer->producePackets(unit.data);
           // only decrease sentData of the amount actually transmitted.
           auto consumedData = unit.data - app->getScheduleData();
           emit(consumedData_s, consumedData.get());
           EV_INFO << LOG_MOD << " consumed " << consumedData.str() << " from " << unit.data << " scheduled" << endl;
           sentData = sentData - consumedData;
       } else {
           emit(consumedData_s, (long)0);
           EV_INFO << LOG_MOD << " No data in application. Scheduled budget dropped" << endl;
       }
       app->setScheduleData(b(0)); // reset scheduledData
    }
}


void IntervalScheduler::scheduleApp(cMessage *message){
    Enter_Method("scheduleApp");
    Unit u = getScheduledUnit();

    switch (u.type) {
        case IntervalSchedulerType::PACKET:
            scheduleByPackets(u);
            break;
        case IntervalSchedulerType::DATA:
            scheduleByData(u);
            break;
        default:
            throw cRuntimeError("expected packet or data scheduling");
    }


}


}
