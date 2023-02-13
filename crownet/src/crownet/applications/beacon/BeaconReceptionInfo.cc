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

uint32_t BeaconReceptionInfo::get32BitTimestamp(omnetpp::simtime_t time) const {
    // mod 32
    // simtime_t in ms (uint64_t) sentTim only has unit32_t thus time mod 32
    return (uint32_t)(simTime().inUnit(SimTimeUnit::SIMTIME_MS) & ((uint64_t)(1) << 32)-1);
}


std::string BeaconReceptionInfo::str() const {
    std::stringstream s;
    s.precision(4);
    auto age = simTime() - currentData->getReceivedTime();
    s << "{id: " << getNodeId() \
            << " tx_t:" << currentData->getReceivedTime().ustr()  \
            << " age:" << age.ustr() << " jitter:" << jitter.ustr() \
            << " avg_s:" << avg_packet_size.str() \
            << " count:" << packetsReceivedCount \
            << " loss_r:" <<  packetLossRate << "}";
    return s.str();
}

std::string BeaconReceptionInfo::logShort() const {
    std::stringstream s;
    if (getCurrentData() == nullptr){
            s << "Beacon[id:" << getNodeId() << ", no-data!]";
        } else {
            auto d = getCurrentData();
            auto p = getPrioData();
            auto prioSeq = (p == nullptr) ? 0 : p->getSequenceNumber();
            s << "Beacon[id:" << getNodeId() << ", SeqNo.:" << prioSeq << "|" << d->getSequenceNumber() \
                        << ", rxTime: " << d->getReceivedTime().ustr() \
                        << ", pos.:" << d->getPosition() << "]";
        }
    return s.str();
}

std::string BeaconReceptionInfo::infoStrShort() const{
    std::stringstream s;
    s << "nodeId: " << getNodeId();
    if (getCurrentData() == nullptr){
        s << ", no-data! ";
    } else {
        auto d = getCurrentData();
        s << ", rxTime: " << d->getReceivedTime().ustr() \
                << ", maxSeqNo.:" << getMaxSequenceNumber() \
                << ", pos.:" << d->getPosition() \
                << ", No.Neighbours: " << d->getNumberOfNeighbours();
    }
    return s.str();
}

void BeaconReceptionInfo::updateCurrentPktInfo(const Packet *packetIn, const int rcvStationId, const simtime_t arrivalTime){
    auto beacon = dynamicPtrCast<const DynamicBeaconPacket>(packetIn->peekData());
    swapAndGetCurrentPktInfo();

    currentPkt->setSourceId(beacon->getSourceId());
    auto creationSimTime = timestamp_32_ms_to_simtime(beacon->getTimestamp(), simTime());
    currentPkt->setCreationTime(creationSimTime);
    currentPkt->setCreationTimeStamp(beacon->getTimestamp());
    currentPkt->setSequenceNumber(beacon->getSequenceNumber());
    currentPkt->setReceivedTime(arrivalTime);
}

void BeaconReceptionInfo::processInbound(const Packet *packetIn,
        const int rcvStationId,
        const simtime_t arrivalTime){
    auto beacon = dynamicPtrCast<const DynamicBeaconPacket>(packetIn->peekData());
    updateCurrentPktInfo(packetIn, rcvStationId, arrivalTime);

    // includes duplicates and out of order!
    computeMetrics(packetIn);


    if (currentPkt->getOutOfOrder()){
        // do not update Beacon Data.
        updated = false; // payload info was not updated
    } else {

        BeaconData* newData;
        if (prioData == nullptr){
            newData = new BeaconData();
            take(newData);
        } else {
            // reuse object
            newData = prioData;
            prioData = nullptr;
        }
//        newData->setBeaconValue(beacon->getTimestamp());
        newData->setBeaconValue(1.0); // Always 1 for this kind of beacon
        newData->setPosition(beacon->getPos());
        newData->setEpsilon(beacon->getEpsilon());
        newData->setNumberOfNeighbours(beacon->getNumberOfNeighbours());
        newData->setSourceId(beacon->getSourceId());
//        newData->setCreationTime(beacon->getTimestamp());
        newData->setCreationTime(timestamp_32_ms_to_simtime(beacon->getTimestamp(), simTime()));
        newData->setSequenceNumber(beacon->getSequenceNumber());
        newData->setReceivedTime(arrivalTime);

        // move data
        prioData = currentData;
        currentData = newData;

        updated = true; // new data
    }
}

bool BeaconReceptionInfo::checkCurrentTtlReached(const omnetpp::simtime_t& ttl) const {
    return (currentData->getCreationTime() + ttl) < simTime();
}

void BeaconReceptionInfo::initAppData() {
    setCurrentData(new BeaconData());
}

bool BeaconReceptionInfo::hasPrio() const {
    return prioData != nullptr;
}

void BeaconReceptionInfo::clearApplicationData() {
    delete removePrioData();
    delete removeCurrentData();
}


} /* namespace crownet */
