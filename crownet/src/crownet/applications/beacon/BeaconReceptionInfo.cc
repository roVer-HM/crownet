/*
 * BeaconAppInfoReceptionInfo.cc
 *
 *  Created on: Aug 30, 2021
 *      Author: vm-sts
 */

#include "BeaconReceptionInfo.h"

namespace crownet {

Register_Class(BeaconReceptionInfo);

BeaconReceptionInfo::~BeaconReceptionInfo() {
    // TODO Auto-generated destructor stub
}

void BeaconReceptionInfo::setSentTimePrio(omnetpp::simtime_t time){
    // mod 32
    // simtime_t in ms (uint64_t) sentTim only has unit32_t thus time mod 32
    sentTimePrio = (uint32_t)(simTime().inUnit(SimTimeUnit::SIMTIME_MS) & ((uint64_t)(1) << 32)-1);
}


std::string BeaconReceptionInfo::str() const {
    std::stringstream s;
    auto age = simTime() - getReceivedTimePrio();
    s << "{id: " << getNodeId() \
            << " a_t:" << getReceivedTimePrio().ustr()  \
            << " age:" << age.ustr() << "}";
    return s.str();
}

void BeaconReceptionInfo::processInbound(Packet *inbound,
        const uint32_t rcvStationId,
        const simtime_t arrivalTime){

    auto header = inbound->popAtFront<DynamicBeaconHeader>();
    // duplicates and out of order!
    ++packetsReceivedCount;

    // first packet. Initialize object
    if (packetsReceivedCount == 1){
        initialSequencenumber = header->getSequencenumber();
        maxSequencenumber = initialSequencenumber;
    }

    bool oldPacket = false;
    // update sequncenumber and cycle
    if (header->getSequencenumber() > maxSequencenumber){
        // ignore late packets from previous wrap if packet_seq > 0xFFEF and max_seq < 0x10
        if (!(header->getSequencenumber() > 0xFFEF && maxSequencenumber < 0x10)){
            maxSequencenumber = header->getSequencenumber();
        } else {
            oldPacket = true; // ignore data due to out of date
        }
    } else {
        if (header->getSequencenumber() < 0x10 && maxSequencenumber > 0xFFEF){
            maxSequencenumber = header->getSequencenumber();
            sequencecycle += 0x00010000;
        } else {
            oldPacket = true; // ignore data due to out of date
        }
    }

    if (maxSequencenumber < initialSequencenumber){
        packetsLossCount = 0xFFFF - initialSequencenumber + maxSequencenumber + 1;
    } else {
        packetsLossCount = maxSequencenumber - initialSequencenumber + 1;
    }
    packetsLossCount = packetsLossCount + sequencecycle - packetsReceivedCount;


    // calculate interarrival jitter
    if (clockRate != 0) {
        // timesmap unit32_t field in ms
        simtime_t d =  SimTime(header->getTimestamp() - sentTimePrio, SimTimeUnit::SIMTIME_MS) - (arrivalTime - receivedTimePrio);
        if (d < 0)
            d = -d;
        jitter = jitter + (d - jitter) / 16;
    }

    // update timestamp
    sentTimePrio = header->getTimestamp();
    receivedTimePrio = arrivalTime;



    if (!oldPacket){
        // only update beacon data if the packet is the news received. Do not
        // use data from old out of order packets.
        auto beacon = inbound->popAtFront<DynamicBeaconPacket>();
        pos = beacon->getPos();
        epsilon = beacon->getEpsilon();
        numberOfNeighbours = beacon->getNumberOfNeighbours();
    }
}


} /* namespace crownet */
