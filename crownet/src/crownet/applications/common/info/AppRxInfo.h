/*
 * AppRxInfoX.h
 *
 *  Created on: Feb 7, 2023
 *      Author: vm-sts
 */

#ifndef CROWNET_APPLICATIONS_COMMON_INFO_APPRXINFOX_H_
#define CROWNET_APPLICATIONS_COMMON_INFO_APPRXINFOX_H_

#include "crownet/applications/common/info/AppInfo_m.h"

namespace crownet {

class AppRxInfo : public AppRxInfo_Base {
public:
    AppRxInfo(const char *name=nullptr): AppRxInfo_Base(name){};
    virtual ~AppRxInfo();

    virtual void computeMetrics(const Packet *packetIn) override;

    virtual PacketInfo* swapAndGetCurrentPktInfo();
    virtual void calcJitter();
    virtual void calcAvgPacketSize(const Packet *packetIn);

};

} /* namespace crownet */

#endif /* CROWNET_APPLICATIONS_COMMON_INFO_APPRXINFOX_H_ */
