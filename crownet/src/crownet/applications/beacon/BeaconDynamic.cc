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

}

// cSimpleModule
void BeaconDynamic::initialize(int stage) {
    BaseApp::initialize(stage);
    if (stage == INITSTAGE_LOCAL){
        nTable = inet::getModuleFromPar<INeighborhoodTable>(par("neighborhoodTableMobdule"), inet::getContainingNode(this));

        minSentFrequency = par("minSentFrequency");
        maxSentFrequyncy = par("maxSentFrequency");
        maxBandwith = par("maxBandwith");
    }
}


Packet *BeaconDynamic::createPacket() {
    const auto& header = makeShared<DynamicBeaconHeader>();
    header->setSequencenumber(localInfo->nextSequenceNumber());
    header->setSourceId(getHostId());
    // mod 32
    uint32_t time = (uint32_t)(simTime().inUnit(SimTimeUnit::SIMTIME_MS) & ((uint64_t)(1) << 32)-1);
    header->setTimestamp(time);


    const auto &beacon = makeShared<DynamicBeaconPacket>();
    beacon->setPos(getPosition());
    beacon->setEpsilon({0.0, 0.0, 0.0});
    // measurement time is same as packet creation.
    beacon->setPosTimestamp(time);
    beacon->setNumberOfNeighbours(nTable->getSize());

    auto packet = buildPacket(beacon, header);

    // process local for own location entry in neighborhood table.
    auto tmp = packet->dup();
    handleDataArrived(tmp);
    delete tmp;

    return packet;
}


FsmState BeaconDynamic::handleDataArrived(Packet *packet){


    auto info = nTable->getOrCreateEntry(packet->peekAtFront<DynamicBeaconHeader>()->getSourceId());

    info->processInbound(packet, hostId, simTime());
    nTable->emitPostChanged(info);

    return FsmRootStates::WAIT_ACTIVE;
}


} /* namespace crownet */
