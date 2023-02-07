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
    auto age = simTime() - currentData->getReceivedTime();
    s << "{id: " << getNodeId() \
            << " a_t:" << currentData->getReceivedTime().ustr()  \
            << " age:" << age.ustr() << "}";
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

void BeaconReceptionInfo::processInbound(const Packet *packetIn,
        const int rcvStationId,
        const simtime_t arrivalTime){

    auto beacon = dynamicPtrCast<const DynamicBeaconPacket>(packetIn->peekData());
    auto info = swapAndGetCurrentPktInfo();
    info->setSourceId(beacon->getSourceId());
    auto creationSimTime = timestamp_32_ms_to_simtime(beacon->getTimestamp(), simTime());
    info->setCreationTime(creationSimTime);
    info->setCreationTimeStamp(beacon->getTimestamp());
    info->setSequenceNumber(beacon->getSequenceNumber());
    info->setReceivedTime(arrivalTime);

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
