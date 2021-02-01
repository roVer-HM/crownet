/*
 * NeighborhoodTable.cc
 *
 *  Created on: Jan 23, 2021
 *      Author: sts
 */

#include "NeighborhoodTable.h"
#include <omnetpp/simtime_t.h>
#include <omnetpp/cstlwatch.h>
#include <omnetpp/cwatch.h>


using namespace omnetpp;
using namespace inet;

namespace crownet {

Define_Module(NeighborhoodTable);

NeighborhoodTable::~NeighborhoodTable(){
    if (ttl_msg != nullptr)
        cancelAndDelete(ttl_msg);
}

// cSimpleModule
void NeighborhoodTable::initialize(int stage){
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL){
        maxAge = par("maxAge");
        ttl_msg = new cMessage("NeighborhoodTable_ttl");
        scheduleAt(simTime() + maxAge, ttl_msg);
        WATCH_MAP(_table);
        WATCH(maxAge);
    }
}

void NeighborhoodTable::handleMessage(cMessage *msg){
    if (msg == ttl_msg){
        checkTimeToLive();
        scheduleAt(simTime() + maxAge, ttl_msg);
    } else {
        throw cRuntimeError("Unknown Message received");
    }
}

void NeighborhoodTable::handleBeacon(NeighborhoodTableEntry&& beacon){
    Enter_Method_Silent();
    const auto& iter = _table.find(beacon.getNodeId());
    if (iter != _table.end() && iter->second.getTimeSend() > beacon.getTimeSend()){
        // ignore received beacon is older than beacon already in map
        return;
    }
    _table[beacon.getNodeId()] = std::move(beacon);
}

void NeighborhoodTable::checkTimeToLive(){
    simtime_t now = simTime();
    for( auto it=_table.cbegin(); it !=_table.cend();){
        // Received + maxAge := time at which entry must be removed.
        if ((it->second.getTimeReceived() + maxAge) < now){
           it = _table.erase(it);
        } else {
            ++it;
        }
    }
}


} /* namespace crownet */
