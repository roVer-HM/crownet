/*
 * NeighborhoodTable.cc
 *
 *  Created on: Jan 23, 2021
 *      Author: sts
 */

#include "crownet/neighbourhood/NeighborhoodTable.h"

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
    for(auto entry : _table){
        delete entry.second;
    }
}

// cSimpleModule
void NeighborhoodTable::initialize(int stage){
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL){
        maxAge = par("maxAge");
        ttl_msg = new cMessage("NeighborhoodTable_ttl");
        scheduleAt(simTime() + maxAge, ttl_msg);
        WATCH_PTRMAP(_table);
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

BeaconReceptionInfo* NeighborhoodTable::getOrCreateEntry(const int sourceId){
    Enter_Method_Silent();
    if (_table.find(sourceId) == _table.end()){
        BeaconReceptionInfo* info = new BeaconReceptionInfo();
        info->setNodeId(sourceId);
        take(info);
        _table[sourceId] = info;
    }
    return _table[sourceId];
}


void NeighborhoodTable::checkTimeToLive(){
    Enter_Method_Silent();
    simtime_t now = simTime();
    if (now >lastCheck){
        for( auto it=_table.cbegin(); it !=_table.cend();){
            // Received + maxAge := time at which entry must be removed.
            if ((it->second->getReceivedTimePrio() + maxAge) < now){
                delete it->second;
                it = _table.erase(it);
            } else {
                ++it;
            }
        }
        lastCheck = now;
    }
}

const int NeighborhoodTable::getNeighbourCount(){
    Enter_Method_Silent();
    checkTimeToLive();
    return _table.size();
}


} /* namespace crownet */
