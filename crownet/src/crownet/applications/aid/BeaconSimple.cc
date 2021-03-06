/*
 * BeaconSimple.cc
 *
 *  Created on: Jan 23, 2021
 *      Author: sts
 */

#include "BeaconSimple.h"
#include "crownet/applications/Beacon_m.h"
#include "inet/common/ModuleAccess.h"

namespace crownet {

Define_Module(BeaconSimple)

BeaconSimple::BeaconSimple() {}

// cSimpleModule
void BeaconSimple::initialize(int stage) {
    AidBaseApp::initialize(stage);
    if (stage == INITSTAGE_LOCAL){
        mobility = inet::getModuleFromPar<inet::IMobility>(par("mobilityModule"), inet::getContainingNode(this));
        nTable = inet::getModuleFromPar<NeighborhoodTable>(par("neighborhoodTableMobdule"), inet::getContainingNode(this));
        hostId = getContainingNode(this)->getId();
        WATCH(hostId);
    }
}

// FSM
void BeaconSimple::setupTimers() {
    if (destAddresses.empty()) {
      cRuntimeError("No address set.");
    } else {
      // schedule at startTime or current time, whatever is bigger.
      scheduleNextAppMainEvent(std::max(startTime, simTime()));
    }
}

BaseApp::FsmState BeaconSimple::fsmAppMain(cMessage *msg) {

    const auto &beacon = createPacket<BeaconPacket>(B(224)); //64+64+64+32
    beacon->setTime(simTime());
    beacon->setPos(mobility->getCurrentPosition());
    beacon->setNodeId(hostId);

    sendPayload(beacon);
    scheduleNextAppMainEvent();
    return FsmRootStates::WAIT_ACTIVE;
}


void BeaconSimple::setAppRequirements(){
    L3Address destAddr = chooseDestAddr();
    socket.setAppRequirements(1.0, 2.0, AidRecipientClass::ALL_LOCAL, destAddr,
                              destPort);
}
void BeaconSimple::setAppCapabilities(){
    // todo: no CAP right now.
}

BaseApp::FsmState BeaconSimple::handleSocketDataArrived(Packet *packet){
    auto p = checkEmitGetReceived<BeaconPacket>(packet);

    NeighborhoodTableEntry b{p->getNodeId(), p->getTime(), simTime(), p->getPos(), p->getEpsilon()};
    nTable->handleBeacon(std::move(b));
    return FsmRootStates::WAIT_ACTIVE;
}


} /* namespace crownet */
