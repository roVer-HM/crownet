/*
 * BeaconSimple.cc
 *
 *  Created on: Jan 23, 2021
 *      Author: sts
 */

#include "crownet/applications/beacon/BeaconSimple.h"
#include "crownet/applications/beacon/Beacon_m.h"
#include "inet/common/ModuleAccess.h"

namespace crownet {

Define_Module(BeaconSimple)

BeaconSimple::BeaconSimple() {}

// cSimpleModule
void BeaconSimple::initialize(int stage) {
    BaseApp::initialize(stage);
    if (stage == INITSTAGE_LOCAL){
        mobility = inet::getModuleFromPar<inet::IMobility>(par("mobilityModule"), inet::getContainingNode(this));
        nTable = inet::getModuleFromPar<NeighborhoodTable>(par("neighborhoodTableMobdule"), inet::getContainingNode(this));
    }
}

Packet *BeaconSimple::createPacket() {

    auto beacon = createPayload<BeaconPacketSimple>();
    beacon->setTime(simTime());
    beacon->setPos(mobility->getCurrentPosition());
    beacon->setNodeId(getHostId());

    return buildPacket(beacon);
}

FsmState BeaconSimple::handleDataArrived(Packet *packet){
    auto p = packet->popAtFront<BeaconPacketSimple>();

    auto info = nTable->getOrCreateEntry(p->getNodeId());
    info->setSentTimePrio(p->getTime());
    info->setReceivedTimePrio(simTime());
    info->setPos(p->getPos());
    info->setEpsilon(p->getEpsilon());

    return FsmRootStates::WAIT_ACTIVE;
}


} /* namespace crownet */
