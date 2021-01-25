/*
 * BeaconSimple.h
 *
 *  Created on: Jan 23, 2021
 *      Author: sts
 */

#pragma once

#include "inet/common/InitStages.h"
#include "inet/mobility/contract/IMobility.h"
#include "crownet/applications/common/AidBaseApp.h"
#include "crownet/common/NeighborhoodTable.h"

#include "artery/utility/IdentityRegistry.h"

namespace crownet {

class BeaconSimple : public AidBaseApp {
public:
    BeaconSimple();
    virtual ~BeaconSimple() = default;

    // cSimpleModule
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    // FSM
    virtual void setupTimers() override;
    virtual FsmState fsmAppMain(cMessage *msg) override;

    // Aid Socket
    virtual void setAppRequirements() override;
    virtual void setAppCapabilities() override;
    virtual void socketDataArrived(AidSocket *socket, Packet *packet) override;


private:
    inet::IMobility *mobility = nullptr;
    NeighborhoodTable *nTable  = nullptr;
    int hostId;
};

} /* namespace crownet */

