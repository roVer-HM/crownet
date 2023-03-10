/*
 * AppRxInfoX.h
 *
 *  Created on: Feb 7, 2023
 *      Author: vm-sts
 */

#ifndef CROWNET_APPLICATIONS_COMMON_INFO_APPRXINFOX_H_
#define CROWNET_APPLICATIONS_COMMON_INFO_APPRXINFOX_H_

#include "crownet/applications/common/info/AppInfo_m.h"
#include "crownet/common/BurstIdSet.h"

namespace crownet {

class AppRxInfo : public AppRxInfo_Base {
public:
    virtual ~AppRxInfo();
    AppRxInfo(const char *name=nullptr): AppRxInfo_Base(name){};
    AppRxInfo(const AppRxInfo& other);
    virtual AppRxInfo *dup() const override {return new AppRxInfo(*this);};

    // override for granular handling of packet types
    virtual void processInbound(Packet *packetIn, const int rcvStationId,
            const simtime_t arrivalTime) override;
    virtual void updateCurrentPktInfo(Packet *packetIn, const int rcvStationId,
            const simtime_t arrivalTime);
    virtual PacketInfo* swapAndGetCurrentPktInfo();


    virtual void computeMetrics(Packet *packetIn) override;


    virtual void calcJitter();
    virtual void calcAvgPacketSize(Packet *packetIn);


private:
    void copy(const AppRxInfo& other);

private:
    BurstIdSet burstIdSet;
};

} /* namespace crownet */

#endif /* CROWNET_APPLICATIONS_COMMON_INFO_APPRXINFOX_H_ */
