/*
 * BeaconDynamic.h
 *
 *  Created on: Aug 30, 2021
 *      Author: vm-sts
 */

#pragma once

#include "crownet/applications/common/BaseApp.h"
#include "crownet/neighbourhood/NeighborhoodTable.h"
#include "inet/mobility/contract/IMobility.h"


namespace crownet {

class BeaconDynamic: public BaseApp {
public:
    virtual ~BeaconDynamic();

    // cSimpleModule
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    //CrownetPacketSourceBase
    virtual Packet *createPacket() override;
    virtual BurstInfo getBurstInfo(inet::b) const override;


    // FSM
    virtual FsmState handleDataArrived(Packet *packet) override;

private:
    INeighborhoodTable* nTable = nullptr;
    INeighborhoodTablePacketProcessor* tablePktProcessor = nullptr;
    double minSentFrequency;
    double maxSentFrequyncy;
    double maxBandwidth;
};

} /* namespace crownet */

