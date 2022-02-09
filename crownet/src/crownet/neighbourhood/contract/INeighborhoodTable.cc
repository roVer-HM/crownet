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

void INeighborhoodTable::emitPreChanged(BeaconReceptionInfo* oldInfo){
    for(auto e : this->listeners){
        e->neighborhoodEntryPreChanged(this, oldInfo);
    }
}
void INeighborhoodTable::emitPostChanged(BeaconReceptionInfo* newInfo){
    for(auto e : this->listeners){
        e->neighborhoodEntryPostChanged(this, newInfo);
    }
}
void INeighborhoodTable::emitRemoved(BeaconReceptionInfo* info){
    for(auto e : this->listeners){
        e->neighborhoodEntryRemoved(this, info);
    }
}

}
