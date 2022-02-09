/*
 * BeaconSimple.h
 *
 *  Created on: Jan 23, 2021
 *      Author: sts
 */

#pragma once

#include "inet/common/InitStages.h"
#include "inet/mobility/contract/IMobility.h"
#include "crownet/applications/common/BaseApp.h"

#include "artery/utility/IdentityRegistry.h"

#include "crownet/neighbourhood/NeighborhoodTable.h"

namespace crownet {

class BeaconSimple : public BaseApp {
public:
    BeaconSimple();
    virtual ~BeaconSimple() = default;

    // cSimpleModule
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    //CrownetPacketSourceBase
    virtual Packet *createPacket() override;

    // FSM
    virtual FsmState handleDataArrived(Packet *packet) override;


private:
    NeighborhoodTable *nTable  = nullptr;
};

} /* namespace crownet */

