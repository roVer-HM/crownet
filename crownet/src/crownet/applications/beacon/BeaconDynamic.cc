/*
 * BeaconDynamic.cc
 *
 *  Created on: Aug 30, 2021
 *      Author: vm-sts
 */

#include "crownet/applications/beacon/BeaconDynamic.h"
#include "crownet/applications/beacon/Beacon_m.h"

namespace crownet {

Define_Module(BeaconDynamic);

BeaconDynamic::~BeaconDynamic() {
    // TODO Auto-generated destructor stub
}

// cSimpleModule
void BeaconDynamic::initialize(int stage) {
    BaseApp::initialize(stage);
    if (stage == INITSTAGE_LOCAL){
        mobility = inet::getModuleFromPar<inet::IMobility>(par("mobilityModule"), inet::getContainingNode(this));
        nTable = inet::getModuleFromPar<NeighborhoodTable>(par("neighborhoodTableMobdule"), inet::getContainingNode(this));
        hostId = (uint32_t)getContainingNode(this)->getId();
        WATCH(hostId);
    }
}

void BeaconDynamic::scheduleNextAppMainEvent(simtime_t time){
    // todo: use dynamic interval alg based on number of participants.
    BaseApp::scheduleNextAppMainEvent(time);
}


// FSM
FsmState BeaconDynamic::fsmAppMain(cMessage *msg) {

    const auto& header = makeShared<DynamicBeaconHeader>();
    header->setSequencenumber(localInfo->nextSequenceNumber());
    header->setSourceId(hostId);
    // mod 32
    uint32_t time = (uint32_t)(simTime().inUnit(SimTimeUnit::SIMTIME_MS) & ((uint64_t)(1) << 32)-1);
    header->setTimestamp(time);


    const auto &beacon = makeShared<DynamicBeaconPacket>();
    beacon->setPos(mobility->getCurrentPosition());
    beacon->setEpsilon({0.0, 0.0});
    // assume position measurement time is same as packet creation.
    beacon->setPosTimestamp(time);
    // todo access neighbourhood table.
    beacon->setNumberOfNeighbours(3);
    beacon->addTag<CreationTimeTag>()->setCreationTime(simTime());

    Packet *pk = new Packet(localInfo->packetName(packetName).c_str());
    pk->insertAtBack(header);
    pk->insertAtBack(beacon);

    sendPayload(pk);

    scheduleNextAppMainEvent();
    return FsmRootStates::WAIT_ACTIVE;
}

FsmState BeaconDynamic::handleDataArrived(Packet *packet){

    auto info = nTable->getOrCreateEntry(packet->peekAtFront<DynamicBeaconHeader>()->getSourceId());

    info->processInbound(packet, hostId, simTime());

    return FsmRootStates::WAIT_ACTIVE;
}


} /* namespace crownet */
