/*
 * AppInfoReception.cc
 *
 *  Created on: Aug 27, 2021
 *      Author: vm-sts
 */

#include "AppInfoReception.h"

namespace crownet {

AppRxInfoPerSource::~AppRxInfoPerSource() {
    // TODO Auto-generated destructor stub
}

PacketInfo* AppRxInfoPerSource::swapAndGetCurrentPktInfo(){

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
    return currentPkt;
}

void AppRxInfoPerSource::calculatedMetrics(){
    calcJitter();
    checkOutOfOrder();
    calcPacketLoss();
}

void AppRxInfoPerSource::calcJitter(){
    // jitter
    simtime_t delay_delta = ((prioPkt == nullptr) ? 0 : prioPkt->delay())- currentPkt->delay();
    if (delay_delta < 0)
        delay_delta = -delay_delta;

    jitter = jitter + (delay_delta -jitter) / 16;

}

void AppRxInfoPerSource::calcPacketLoss(){
    // [0-----------------------------------------------0xFFFF]
    // [**************^--<maxSeqNo>  <initSeqNo>--^***********]
    int inCyclePackageSendCount;
    if (maxSequenceNumber < initialSequenceNumber){
        // [0-----------------------------------------------0xFFFF]
        // [**************^--<maxSeqNo>  <initSeqNo>--^***********]
        // received packet count in current cycle given by '***' intervals
        inCyclePackageSendCount = 0xFFFF - initialSequenceNumber + maxSequenceNumber + 1;
    } else {
        // [0-----------------------------------------------0xFFFF]
        // [    <initSeqNo>--^**************^--<maxSeqNo>         ]
        // received packet count in current cycle given by '***' interval
        inCyclePackageSendCount = maxSequenceNumber - initialSequenceNumber + 1;
    }
    //todo negativ?
    packetsLossCount = (sequencecycle + inCyclePackageSendCount) - packetsReceivedCount;
}

void AppRxInfoPerSource::checkOutOfOrder(){
    // packet order
    if (packetsReceivedCount == 1){
        // first packet of this source. Init structure
        initialSequenceNumber = currentPkt->getSequenceNumber();
        maxSequenceNumber = initialSequenceNumber;
        sequencecycle = 0;
        currentPkt->setOutOfOrder(false);
    } else {
        // update sequncenumber and cycle
       
        if(currentPkt->getSequenceNumber() > maxSequenceNumber){
            if (currentPkt->getSequenceNumber() > 0xFFEF && maxSequenceNumber < 0x10){
                // [0-----0x10---------------------------0xFFEF-----0xFFFF]
                //     ^--<maxSeqNo>          <currentPktSeqNo>--^
                //  Large gap between maxSeqNo and currentPktSeqNo.
                //  Assume that a large SeqNo is an old packet because maxSeqNo cycled recently to small value          
                currentPkt->setOutOfOrder(true);
            } else {
                maxSequenceNumber = currentPkt->getSequenceNumber();
                currentPkt->setOutOfOrder(false);
            }
        } else {
            if (currentPkt->getSequenceNumber() < 0x10 && maxSequenceNumber > 0xFFEF){
                // [0-----0x10---------------------------0xFFEF-----0xFFFF]
                //     ^--<currentPktSeqNo>          <maxSeqNo>--^
                // SeqNo wrap happend. Distance over the wrap is small therefore assum overflow and
                // Increment sequencecycle (unit_32 vs uint_16)
                maxSequenceNumber = currentPkt->getSequenceNumber();
                sequencecycle += 0x00010000;
                currentPkt->setOutOfOrder(false);
            } else {
                currentPkt->setOutOfOrder(true);
            }
        }
    }
}


} /* namespace crownet */
