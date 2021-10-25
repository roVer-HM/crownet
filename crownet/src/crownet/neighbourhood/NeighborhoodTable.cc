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
#include "crownet/crownet.h"
#include "crownet/common/GlobalDensityMap.h"
#include "crownet/common/util/FileWriterRegister.h"
#include "inet/common/ModuleAccess.h"
#include "crownet/common/IDensityMapHandler.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"

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
    MobilityProviderMixin<cSimpleModule>::initialize(stage);
    if (stage == INITSTAGE_LOCAL){
        maxAge = par("maxAge");
        ttl_msg = new cMessage("NeighborhoodTable_ttl");
        scheduleAt(simTime() + maxAge, ttl_msg);
        WATCH_PTRMAP(_table);
        WATCH(maxAge);
    } else if (stage == INITSTAGE_APPLICATION_LAYER){
        cStringTokenizer t(par("fileWriterRegister").stringValue(), ";");
        auto t_value = t.asVector();
        if (t_value.size() == 2){
            auto fRegister = dynamic_cast<IFileWriterRegister*>(findModuleByPath(t_value[0].c_str()));
            if (fRegister) {
                auto l = fRegister->getWriterAs<NeighborhoodEntryListner>(t_value[1]);
                registerEntryListner(l);
            }
        }
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
        return info;
    } else {
        emitPreChanged(_table[sourceId]);
        return _table[sourceId];
    }
}


void NeighborhoodTable::checkTimeToLive(){
    Enter_Method_Silent();
    simtime_t now = simTime();
    if (now >lastCheck){
        // remove old entries
        for( auto it=_table.cbegin(); it !=_table.cend();){
            // Received + maxAge := time at which entry must be removed.
            if ((it->second->getReceivedTimePrio() + maxAge) < now){
                emitRemoved(it->second);
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
