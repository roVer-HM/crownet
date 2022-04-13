/*
 * BeaconDynamic.cc
 *
 *  Created on: Aug 30, 2021
 *      Author: vm-sts
 */

#include "crownet/applications/beacon/BeaconDynamic.h"
#include "crownet/applications/beacon/Beacon_m.h"
#include "crownet/crownet.h"

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
    const auto &beacon = makeShared<DynamicBeaconPacket>();

    beacon->setSequencenumber(localInfo->nextSequenceNumber());
    beacon->setSourceId(getHostId());
    uint32_t time = simtime_to_timestamp_32_ms();
    beacon->setTimestamp(time);


    beacon->setPos(getPosition());
    beacon->setEpsilon({0.0, 0.0, 0.0});
    beacon->setNumberOfNeighbours(nTable->getSize());

    auto packet = buildPacket(beacon);

    // process local for own location entry in neighborhood table.
    auto tmp = packet->dup();
    handleDataArrived(tmp);
    delete tmp;

    return packet;
}


FsmState BeaconDynamic::handleDataArrived(Packet *packet){

    auto pSrcId = packet->peekAtFront<DynamicBeaconPacket>()->getSourceId();
    auto info = nTable->getOrCreateEntry(pSrcId);
    // process new beacon
    info->processInbound(packet, hostId, simTime());
    nTable->processInfo(info);

    return FsmRootStates::WAIT_ACTIVE;
}


} /* namespace crownet */
