/*
 * AppInfoReception.h
 *
 *  Created on: Aug 27, 2021
 *      Author: vm-sts
 */

#ifndef CROWNET_APPLICATIONS_COMMON_INFO_APPINFORECEPTION_H_
#define CROWNET_APPLICATIONS_COMMON_INFO_APPINFORECEPTION_H_

#include "AppInfo_m.h"
#include "inet/common/packet/Packet.h"

using namespace inet;

namespace crownet {

class AppRxInfoPerSource: public AppRxInfoPerSource_Base {
public:
    virtual ~AppRxInfoPerSource();
    AppRxInfoPerSource(const char *name=nullptr) : AppRxInfoPerSource_Base(name) {}


    // override for granular handling of packet types
    virtual void processInbound(const Ptr<const Chunk> dataInbound, const uint32_t rcvStationId,
            const simtime_t arrivalTime) = 0;

    virtual PacketInfo* swapAndGetCurrentPktInfo();
    virtual void calculatedMetrics();
    virtual void calcJitter();
    virtual void calcPacketLoss();
    virtual void checkOutOfOrder();
};

} /* namespace crownet */

#endif /* CROWNET_APPLICATIONS_COMMON_INFO_APPINFORECEPTION_H_ */
