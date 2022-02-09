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

class AppInfoReception: public AppInfoReception_Base {
public:
    virtual ~AppInfoReception();
    AppInfoReception(const char *name=nullptr) : AppInfoReception_Base(name) {}


    // override for granular handling of packet types
    virtual void processInbound(Packet *inbound, const uint32_t rcvStationId,
            const simtime_t arrivalTime) = 0;
};

} /* namespace crownet */

#endif /* CROWNET_APPLICATIONS_COMMON_INFO_APPINFORECEPTION_H_ */
