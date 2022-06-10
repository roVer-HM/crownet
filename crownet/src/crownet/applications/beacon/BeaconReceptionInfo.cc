/*
 * BeaconAppInfoReceptionInfo.cc
 *
 *  Created on: Aug 30, 2021
 *      Author: vm-sts
 */

#include "BeaconReceptionInfo.h"
#include "crownet/crownet.h"

namespace crownet {

Register_Class(BeaconReceptionInfo);

BeaconReceptionInfo::~BeaconReceptionInfo() {
    // TODO Auto-generated destructor stub
}

void BeaconReceptionInfo::setSentTimeCurrent(omnetpp::simtime_t time){
    // mod 32
    // simtime_t in ms (uint64_t) sentTim only has unit32_t thus time mod 32
    sentTimeCurrent = (uint32_t)(simTime().inUnit(SimTimeUnit::SIMTIME_MS) & ((uint64_t)(1) << 32)-1);
}


std::string BeaconReceptionInfo::str() const {
    std::stringstream s;
    auto age = simTime() - getReceivedTimeCurrent();
    s << "{id: " << getNodeId() \
            << " a_t:" << getReceivedTimeCurrent().ustr()  \
            << " age:" << age.ustr() << "}";
    return s.str();
}

void BeaconReceptionInfo::processInbound(Packet *inbound,
        const uint32_t rcvStationId,
        const simtime_t arrivalTime){

    auto beacon = inbound->popAtFront<DynamicBeaconPacket>();
    // includes duplicates and out of order!
    ++packetsReceivedCount;

    // calculate interarrival jitter (includes duplicates and out of order!)
    if (clockRate != 0) {
        // timesmap unit32_t field in ms
        simtime_t d =  SimTime(beacon->getTimestamp() - sentTimeCurrent, SimTimeUnit::SIMTIME_MS) - (arrivalTime - receivedTimeCurrent);
        if (d < 0)
            d = -d;
        jitter = jitter + (d - jitter) / 16;
    }

    bool oldPacket = false;

    if (packetsReceivedCount == 1){
        // first packet. Initialize object
        initialSequencenumber = beacon->getSequencenumber();
        maxSequencenumber = initialSequencenumber;
        sequencecycle = 0;
        oldPacket = false;

    } else {
        // update sequncenumber and cycle
        if (beacon->getSequencenumber() > maxSequencenumber){
            // ignore late packets from previous wrap if packet_seq > 0xFFEF and max_seq < 0x10
            if (!(beacon->getSequencenumber() > 0xFFEF && maxSequencenumber < 0x10)){
                maxSequencenumber = beacon->getSequencenumber();
                oldPacket = false;
            } else {
                oldPacket = true; // ignore data due to out of date
            }
        } else {
            if (beacon->getSequencenumber() < 0x10 && maxSequencenumber > 0xFFEF){
                maxSequencenumber = beacon->getSequencenumber();
                sequencecycle += 0x00010000;
                oldPacket = false;
            } else {
                oldPacket = true; // ignore data due to out of date
            }
        }
    }

    if (maxSequencenumber < initialSequencenumber){
        packetsLossCount = 0xFFFF - initialSequencenumber + maxSequencenumber + 1;
    } else {
        packetsLossCount = maxSequencenumber - initialSequencenumber + 1;
    }
    packetsLossCount = packetsLossCount + sequencecycle - packetsReceivedCount;


    if (oldPacket){
        // do not update payload info.
        updated = false; // payload info was not updated
    } else {
        // save previous values
        receivedTimePrio = receivedTimeCurrent;
        sentTimePrio = sentTimeCurrent;
        sentSimTimePrio = sentSimTimeCurrent;
        packetsReceivedCountPrio = packetsReceivedCountCurrent;
        positionPrio = positionCurrent;
        epsilonPrio = epsilonCurrent;
        numberOfNeighboursPrio = numberOfNeighboursCurrent;
        beaconValuePrio = beaconValueCurrent;

        // update timestamp
        sentTimeCurrent = beacon->getTimestamp();
        sentSimTimeCurrent = timestamp_32_ms_to_simtime(beacon->getTimestamp(), simTime());
        receivedTimeCurrent = arrivalTime;

        // only update beacon data if the packet is the news received. Do not
        // use data from old out of order packets.
        positionCurrent = beacon->getPos();
        epsilonCurrent = beacon->getEpsilon();
        numberOfNeighboursCurrent = beacon->getNumberOfNeighbours();
        beaconValueCurrent = 1.0; // Always 1 for this kind of beacon

        updated = true; // new data
    }
}

bool BeaconReceptionInfo::checkCurrentTtlReached(const omnetpp::simtime_t& ttl){
    return getSentSimTimeCurrent() + ttl < simTime();
}

bool BeaconReceptionInfo::checkPrioTtlReached(const omnetpp::simtime_t& ttl){
    if (hasPrio()){
        return getSentSimTimePrio() + ttl < simTime();
    } else {
        return true;
    }
}


} /* namespace crownet */
