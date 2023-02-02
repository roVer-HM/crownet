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
    virtual void processInbound(const Packet *packetIn, const int rcvStationId,
            const simtime_t arrivalTime) override;

    // converts simtime_t time into unit32_t representation (ms)
    virtual uint32_t get32BitTimestamp(omnetpp::simtime_t time) const;

    virtual std::string str() const override;
    virtual std::string infoStrShort() const;

    bool isUpdated() const {return updated;}

    bool checkCurrentTtlReached(const omnetpp::simtime_t& ttl) const ;
    void clearApplicationData();

    void initAppData();

    // more than one valid beacon arrived.
    bool hasPrio() const;

protected:

    // true object was passed to density map
    bool updated;

};

} /* namespace crownet */

