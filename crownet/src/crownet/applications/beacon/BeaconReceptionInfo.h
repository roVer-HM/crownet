/*
 * BeaconAppInfoReceptionInfo.h
 *
 *  Created on: Aug 30, 2021
 *      Author: vm-sts
 */


#pragma once

#include "crownet/applications/beacon/Beacon_m.h"
#include "crownet/common/util/FilePrinter.h"


namespace crownet {

class BeaconReceptionInfo :
        public FilePrinterMixin<BeaconReceptionInfo_Base>
        {
public:
    virtual ~BeaconReceptionInfo();
    BeaconReceptionInfo(const char *name=nullptr)
        : FilePrinterMixin<BeaconReceptionInfo_Base>(name){ }
    BeaconReceptionInfo(const BeaconReceptionInfo& other);

    virtual BeaconReceptionInfo *dup() const override {return new BeaconReceptionInfo(*this);} ;

    // override for granular handling of packet types
    virtual void processInbound(Packet *packetIn, const int rcvStationId,
            const simtime_t arrivalTime) override;
    virtual void updateCurrentPktInfo(Packet *packetIn, const int rcvStationId, const simtime_t arrivalTime) override;


    // converts simtime_t time into unit32_t representation (ms)
    virtual uint32_t get32BitTimestamp(omnetpp::simtime_t time) const;

    virtual std::string str() const override;
    virtual std::string infoStrShort() const;
    virtual std::string logShort() const;

    bool isUpdated() const {return currentPkt != nullptr && !currentPkt->getOutOfOrder();}

    bool checkCurrentTtlReached(const omnetpp::simtime_t& ttl) const ;

    void initAppData();

    // Set currentData of 'other' as prioData of this object.
    void updatePrioAppData(const BeaconReceptionInfo* other);
    bool hasPrio() const;
private:
    void copy(const BeaconReceptionInfo& other);

protected:


};

} /* namespace crownet */

