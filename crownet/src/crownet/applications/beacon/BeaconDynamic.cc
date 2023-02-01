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
        tablePktProcessor = inet::getModuleFromPar<INeighborhoodTablePacketProcessor>(par("neighborhoodTableMobdule"), inet::getContainingNode(this));
        minSentFrequency = par("minSentFrequency");
        maxSentFrequyncy = par("maxSentFrequency");
        maxBandwith = par("maxBandwith");
    }
}


Packet *BeaconDynamic::createPacket() {
    const auto &beacon = makeShared<DynamicBeaconPacket>();

    beacon->setSequenceNumber(localInfo->nextSequenceNumber());
    beacon->setSourceId(getHostId());
    uint32_t time = simtime_to_timestamp_32_ms();
    beacon->setTimestamp(time);


    beacon->setPos(getPosition());
    beacon->setEpsilon({0.0, 0.0, 0.0});
    // nTable contains current node thus #neighbors = size() - 1
    beacon->setNumberOfNeighbours(std::max(0, nTable->getSize()-1));

    auto packet = buildPacket(beacon);

    // no direct call to handleDataArrived needed. Packet is delivered back to sender at the same time.
    return packet;
}


FsmState BeaconDynamic::handleDataArrived(Packet *packet){


    auto pSrcId = packet->peekAtFront<DynamicBeaconPacket>()->getSourceId();
    std::cout << simTime().dbl() << " eventNr. " << getSimulation()->getEventNumber() << " hostID" << hostId << "<---" << pSrcId << std::endl;

    auto infoTag = packet->findTagForUpdate<AppInfoTag>();
    if (infoTag == nullptr){
        throw cRuntimeError("No AppInfoTag found. Application needs an ApplicationMeter to manage AppInfoObjects.");
    }
    auto info = dynamic_cast<BeaconReceptionInfo*>(infoTag->getAppInfoForUpdate());
    if (info == nullptr){
        throw cRuntimeError("Provided AppInfo object cannot be cast to BeaconReceptionInfo");
    }
    tablePktProcessor->processInfo(info);

    return FsmRootStates::WAIT_ACTIVE;
}


} /* namespace crownet */
