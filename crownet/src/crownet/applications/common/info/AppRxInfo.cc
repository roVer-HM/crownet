/*
 * AppRxInfo.cc
 *
 *  Created on: Feb 7, 2023
 *      Author: vm-sts
 */

#include "AppRxInfo.h"
#include "inet/common/TimeTag_m.h"
#include "crownet/applications/common/AppCommon_m.h"


namespace crownet {


AppRxInfo::~AppRxInfo() {
    // TODO Auto-generated destructor stub
}

void AppRxInfo::processInbound(Packet *packetIn, const int rcvStationId, const simtime_t arrivalTime){
    updateCurrentPktInfo(packetIn, rcvStationId, arrivalTime);
    computeMetrics(packetIn);
}


void AppRxInfo::computeMetrics(Packet *packetIn){
    packetsReceivedCount++;
    packetsOctetCount += (int) std::ceil(packetIn->getDataLength().get()/8);
    calcJitter();
    calcAvgPacketSize(packetIn);
}

PacketInfo* AppRxInfo::swapAndGetCurrentPktInfo(){
    auto now = simTime();
    if (lastPktSwap < now){
        PacketInfo* p;
        if (prioPkt == nullptr){
            p = new PacketInfo();
            take(p);
        } else {
            // reuse object
            p = prioPkt;
            prioPkt = nullptr;
        }
        //move current packet back
        prioPkt = currentPkt;
        currentPkt = p;
        lastPktSwap = simTime();
    }
    return currentPkt;
}

void AppRxInfo::updateCurrentPktInfo(Packet *packetIn, const int rcvStationId, const simtime_t arrivalTime){
    auto data = packetIn->peekData();
    swapAndGetCurrentPktInfo();

    currentPkt->setCreationTime(
            data->getAllTags<CreationTimeTag>().front().getTag()->getCreationTime());
    currentPkt->setSequenceNumber(
            data->getAllTags<SequenceIdTag>().front().getTag()->getSequenceNumber());
    currentPkt->setReceivedTime(arrivalTime);
}

void AppRxInfo::calcJitter(){
    // jitter estimation after RTPC
    simtime_t delay_delta = ((prioPkt == nullptr) ? 0 : prioPkt->delay())- currentPkt->delay();
    if (delay_delta < 0)
        delay_delta = -delay_delta;

    // x_1 = a*y + (1-a)*x_0 ==> x_0 + a*y - x_0*a ==> x_0 + (y-x_0)*a
    jitter = jitter + (delay_delta -jitter)*ema_smoothing_jitter;

}

void AppRxInfo::calcAvgPacketSize(Packet *packetIn){

    auto burstTag = packetIn->findRegionTag<BurstTag>(packetIn->getFrontOffset(), packetIn->getDataLength());
    auto burstSkipTag = packetIn->findRegionTag<BurstSkipTag>(packetIn->getFrontOffset(), packetIn->getDataLength());

    if (burstTag == nullptr || burstTag->getBurstPktCount()==1){
        //not a burst packet or only one packet per burst.
        //Just use datalengt.
        // x_1 = a*y + (1-a)*x_0 ==> x_0 + a*y - x_0*a ==> x_0 + (y-x_0)*a
        avg_packet_size = avg_packet_size + (packetIn->getDataLength() - avg_packet_size)*ema_smoothing_packet_size;
        EV_DETAIL << "No burst track packet length." << this->str() << endl;
    } else if (burstSkipTag) {
        // burst is already handled ignore this packet for avgPacketSize calculation.
        // do nothing
        EV_DETAIL << "Skip burst." << this->str() << endl;
    } else {
        if (getNodeId() <= 0){
            // application level object do not check for bursts. This is done by
            // per source object beforehand.
            avg_packet_size = avg_packet_size + (burstTag->getBurstSize() - avg_packet_size)*ema_smoothing_packet_size;
            EV_DETAIL << "PerAppInfo: Found packet belonging to NEW burst. --> Meter packet and. " << burstTag->str() << this->str() << endl;
        } else {
            // per source object handler.
            // At this point it is a Burst Packet with..
            //   At least 2 packets
            if (burstIdSet.contains(burstTag->getBurstCreationTime())){
                // Burst is already known. Do not process this packet
                packetIn->addRegionTagIfAbsent<BurstSkipTag>(packetIn->getFrontOffset(), packetIn->getDataLength());
                EV_DETAIL << "PerSourceInfo: Found packet belonging to previous burst. --> Skip packet. " << burstTag->str() << this->str() << endl;
            } else {
                // First packet of new burst found.
                // Track packet as burst
                burstIdSet.add(burstTag->getBurstCreationTime());
                avg_packet_size = avg_packet_size + (burstTag->getBurstSize() - avg_packet_size)*ema_smoothing_packet_size;
                EV_DETAIL << "PerSourceInfo: Found packet belonging to NEW burst. --> Meter packet and save id. " << burstTag->str() << this->str() <<  endl;
            }
        }
    }
}

} /* namespace crownet */
