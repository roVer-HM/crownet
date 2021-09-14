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


    virtual void scheduleNextAppMainEvent(simtime_t time = -1) override;


    // FSM
    virtual FsmState fsmAppMain(cMessage *msg) override;
    virtual FsmState handleDataArrived(Packet *packet) override;

private:
    inet::IMobility *mobility = nullptr;
    NeighborhoodTable* nTable = nullptr;
    double minSentFrequency;
    double maxSentFrequyncy;
    double maxBandwith;
};

} /* namespace crownet */

