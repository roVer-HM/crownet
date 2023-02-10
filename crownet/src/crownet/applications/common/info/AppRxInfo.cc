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

void AppRxInfo::processInbound(const Packet *packetIn, const int rcvStationId, const simtime_t arrivalTime){
    updateCurrentPktInfo(packetIn, rcvStationId, arrivalTime);
    computeMetrics(packetIn);
}


void AppRxInfo::computeMetrics(const Packet *packetIn){
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

void AppRxInfo::updateCurrentPktInfo(const Packet *packetIn, const int rcvStationId, const simtime_t arrivalTime){
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

void AppRxInfo::calcAvgPacketSize(const Packet *packetIn){

    // x_1 = a*y + (1-a)*x_0 ==> x_0 + a*y - x_0*a ==> x_0 + (y-x_0)*a
    avg_packet_size = avg_packet_size + (packetIn->getDataLength() - avg_packet_size)*ema_smoothing_packet_size;

}

} /* namespace crownet */
