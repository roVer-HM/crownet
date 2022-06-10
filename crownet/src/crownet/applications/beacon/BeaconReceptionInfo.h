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
        : FilePrinterMixin<BeaconReceptionInfo_Base>(name){}


    // override for granular handling of packet types
    virtual void processInbound(Packet *inbound, const uint32_t rcvStationId,
            const simtime_t arrivalTime) override;

    // converts simtime_t time into unit32_t representation (ms)
    using BeaconReceptionInfo_Base::setSentTimeCurrent;
    virtual void setSentTimeCurrent(omnetpp::simtime_t time);

    virtual std::string str() const override;

    bool isUpdated() const {return updated;}

    bool checkCurrentTtlReached(const omnetpp::simtime_t& ttl);
    // will return true if there is no prio packet info
    bool checkPrioTtlReached(const omnetpp::simtime_t& ttl);

protected:

    // true object was passed to density map
    bool updated;

};

} /* namespace crownet */

