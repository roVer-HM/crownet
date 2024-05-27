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

BeaconReceptionInfo::BeaconReceptionInfo(const BeaconReceptionInfo& other)
    :  FilePrinterMixin<BeaconReceptionInfo_Base>(other){
    copy(other);
}

void BeaconReceptionInfo::copy(const BeaconReceptionInfo& other){}


uint32_t BeaconReceptionInfo::get32BitTimestamp(omnetpp::simtime_t time) const {
    // mod 32
    // simtime_t in ms (uint64_t) sentTim only has unit32_t thus time mod 32
    return (uint32_t)(simTime().inUnit(SimTimeUnit::SIMTIME_MS) & ((uint64_t)(1) << 32)-1);
}


std::string BeaconReceptionInfo::str() const {
    std::stringstream s;
    s.precision(4);
    auto d = getCurrentData();
    if (d){
        auto age = simTime() - currentData->getReceivedTime();
        s << "{id: " << getNodeId() \
                << " tx_t:" << currentData->getReceivedTime().ustr()  \
                << " age:" << age.ustr() << " jitter:" << jitter.ustr() \
                << " avg_s:" << avg_packet_size.str() \
                << " count:" << packetsReceivedCount \
                << " loss_r:" <<  packetLossRate << "}";
    } else {
        s << "{id: " << getNodeId() \
                << " tx_t: N/A"  \
                << " age: N/A" << " jitter:" << jitter.ustr() \
                << " avg_s:" << avg_packet_size.str() \
                << " count:" << packetsReceivedCount \
                << " loss_r:" <<  packetLossRate << "}";

    }
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

void BeaconReceptionInfo::updateCurrentPktInfo(Packet *packetIn, const int rcvStationId, const simtime_t arrivalTime){
    auto beacon = dynamicPtrCast<const DynamicBeaconPacket>(packetIn->peekData());
    swapAndGetCurrentPktInfo();

    currentPkt->setSourceId(beacon->getSourceId());
    auto creationSimTime = timestamp_32_ms_to_simtime(beacon->getTimestamp(), simTime());
    currentPkt->setCreationTime(creationSimTime);
    currentPkt->setCreationTimeStamp(beacon->getTimestamp());
    currentPkt->setSequenceNumber(beacon->getSequenceNumber());
    currentPkt->setReceivedTime(arrivalTime);
}

void BeaconReceptionInfo::processInbound(Packet *packetIn,
        const int rcvStationId,
        const simtime_t arrivalTime){
    auto beacon = dynamicPtrCast<const DynamicBeaconPacket>(packetIn->peekData());
    updateCurrentPktInfo(packetIn, rcvStationId, arrivalTime);

    // includes duplicates and out of order!
    computeMetrics(packetIn);


    if (currentPkt->getOutOfOrder()){
        // do not update Beacon Data.
    } else {
        if (currentData == nullptr){
            initAppData();
        }
        currentData->setBeaconValue(1.0); // Always 1 for this kind of beacon
        currentData->setPosition(beacon->getPos());
        currentData->setEpsilon(beacon->getEpsilon());
        currentData->setNumberOfNeighbours(beacon->getNumberOfNeighbours());
        currentData->setSourceId(beacon->getSourceId());
        currentData->setCreationTime(timestamp_32_ms_to_simtime(beacon->getTimestamp(), simTime()));
        currentData->setCreationTimeStamp(beacon->getTimestamp());
        currentData->setSequenceNumber(beacon->getSequenceNumber());
        currentData->setReceivedTime(arrivalTime);
        auto rsdBeacon = dynamicPtrCast<const DynamicBeaconPacketWithSharingDominId>(packetIn->peekData());
        if (rsdBeacon){
            currentData->setRessourceSharingDomainId(rsdBeacon->getSharingDomainId());
        }
    }
}

bool BeaconReceptionInfo::checkCurrentTtlReached(const omnetpp::simtime_t& ttl) const {
    return (currentData->getCreationTime() + ttl) < simTime();
}



void BeaconReceptionInfo::initAppData() {
    setCurrentData(new BeaconData());
}

void BeaconReceptionInfo::updatePrioAppData(const BeaconReceptionInfo* other){
    delete removePrioData();
    if (other != nullptr){
        setPrioData(other->getCurrentData()->dup());
    }
}

bool BeaconReceptionInfo::hasPrio() const {
    return prioData != nullptr;
}


} /* namespace crownet */
