/*
 * INeighborhoodTable.cc
 *
 *  Created on: Oct 21, 2021
 *      Author: vm-sts
 */


#include "crownet/neighbourhood/contract/INeighborhoodTable.h"

namespace crownet {

void INeighborhoodTable::registerEntryListner(NeighborhoodEntryListner* listener){
    this->removeEntryListener(listener);
    this->listeners.push_back(listener);
}

void INeighborhoodTable::registerFirst(NeighborhoodEntryListner* listener){
    this->removeEntryListener(listener);
    this->listeners.push_back(listener);
}


void INeighborhoodTable::removeEntryListener(NeighborhoodEntryListner* listener){
    auto iter = this->listeners.begin();
    auto end = this->listeners.end();
    while(iter != end){
        if (*iter == listener){
          iter = this->listeners.erase(iter);
        } else {
          ++iter;
        }
    }
}

void INeighborhoodTable::emitRemoved(BeaconReceptionInfo* info){
    for(auto e : this->listeners){
        e->neighborhoodEntryRemoved(this, info);
    }
}
void INeighborhoodTable::emitDropped(BeaconReceptionInfo* info){
    for(auto e : this->listeners){
        e->neighborhoodEntryDropped(this, info);
    }
}
//////
void INeighborhoodTable::emitLeaveCell(BeaconReceptionInfo* info){
    for(auto e : this->listeners){
        e->neighborhoodEntryLeaveCell(this, info);
    }
}

void INeighborhoodTable::emitEnterCell(BeaconReceptionInfo* info){
    for(auto e : this->listeners){
        e->neighborhoodEntryEnterCell(this, info);
    }
}

void INeighborhoodTable::emitStayInCell(BeaconReceptionInfo* info){
    for(auto e : this->listeners){
        e->neighborhoodEntryStayInCell(this, info);
    }
}

}
