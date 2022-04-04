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
    using BeaconReceptionInfo_Base::setSentTimePrio;
    virtual void setSentTimePrio(omnetpp::simtime_t time);
    virtual simtime_t getSentSimTime()const {return sentSimTimePrio;}

    virtual std::string str() const override;

protected:
    simtime_t sentSimTimePrio;
};

} /* namespace crownet */

