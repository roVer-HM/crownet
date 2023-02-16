/*
 * BeaconDynamic.cc
 *
 *  Created on: Aug 30, 2021
 *      Author: vm-sts
 */

#include "crownet/applications/common/AppCommon_m.h"
#include "crownet/applications/beacon/BeaconDynamic.h"
#include "crownet/applications/beacon/Beacon_m.h"
#include "crownet/applications/common/info/InfoTags_m.h"
#include "crownet/crownet.h"
#include <cmath>

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
        maxBandwidth = par("maxBandwidth");
    }
}

BurstInfo BeaconDynamic::getBurstInfo(inet::b data) const{
    DynamicBeaconPacket p;
    int pkt_count = (int)std::floor((double)data.get()/p.getChunkLength().get());
    return BurstInfo{pkt_count, inet::b(pkt_count*p.getChunkLength().get())};
}


Packet *BeaconDynamic::createPacket() {
    const auto &beacon = makeShared<DynamicBeaconPacket>();

    auto seqNo = localInfo->nextSequenceNumber();
    beacon->setSequenceNumber(seqNo);
    beacon->addTagIfAbsent<SequenceIdTag>()->setSequenceNumber(seqNo);
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
    auto infoTag = packet->findTagForUpdate<AppRxInfoPerSourceTag>();
    if (infoTag == nullptr){
        throw cRuntimeError("No AppInfoTag found. Application needs an ApplicationMeter to manage AppInfoObjects.");
    }
    auto info = dynamic_cast<BeaconReceptionInfo*>(infoTag->getPacketInfoForUpdate());
    if (info == nullptr){
        throw cRuntimeError("Provided AppInfo object cannot be cast to BeaconReceptionInfo");
    }
    tablePktProcessor->processInfo(info);

    return FsmRootStates::WAIT_ACTIVE;
}


} /* namespace crownet */
