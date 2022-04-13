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
        nTable = inet::getModuleFromPar<NeighborhoodTable>(par("neighborhoodTableMobdule"), inet::getContainingNode(this));
    }
}

Packet *BeaconSimple::createPacket() {

    auto beacon = createPayload<BeaconPacketSimple>();
    beacon->setTime(simTime());
    beacon->setPos(getPosition());
    beacon->setNodeId(getHostId());

    auto packet = buildPacket(beacon);

    // process local for own location entry in neighborhood table.
    auto tmp = packet->dup();
    handleDataArrived(tmp);
    delete tmp;


    return packet;
}

FsmState BeaconSimple::handleDataArrived(Packet *packet){
    auto p = packet->popAtFront<BeaconPacketSimple>();

    auto info = nTable->getOrCreateEntry(p->getNodeId());
    info->setSentTimeCurrent(p->getTime());
    info->setReceivedTimeCurrent(simTime());
    info->setPos(p->getPos());
    info->setEpsilon(p->getEpsilon());
    nTable->emitPostChanged(info);

    return FsmRootStates::WAIT_ACTIVE;
}


} /* namespace crownet */
