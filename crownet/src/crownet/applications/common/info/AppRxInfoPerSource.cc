/*
 * Bar.cc
 *
 *  Created on: Feb 7, 2023
 *      Author: vm-sts
 */

#include "AppRxInfoPerSource.h"

namespace crownet {


AppRxInfoPerSource::~AppRxInfoPerSource() {
    // TODO Auto-generated destructor stub
}

void AppRxInfoPerSource::computeMetrics(const Packet *packetIn){
    AppRxInfo::computeMetrics(packetIn);
    checkOutOfOrder(); // must be before calcPacketLoss()
    calcPacketLoss();
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
    packetsLossCount = std::max(0, packetsLossCount);


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
