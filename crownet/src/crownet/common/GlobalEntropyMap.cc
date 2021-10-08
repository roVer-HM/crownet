//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "GlobalEntropyMap.h"
#include <inet/common/ModuleAccess.h>


using namespace omnetpp;
using namespace inet;

namespace crownet {

Define_Module(GlobalEntropyMap);

void GlobalEntropyMap::initialize(int stage) {
    GlobalDensityMap::initialize(stage);
  if (stage == INITSTAGE_LOCAL) {
      // setup entropy
  }
}

void GlobalEntropyMap::handleMessage(cMessage *msg){
    if (msg == updateTimer){
        // update nTable
        updateEntropy();
    }
    GlobalDensityMap::handleMessage(msg);
}

void GlobalEntropyMap::updateMaps() {
    // update map with nTable
    // do not update localMaps in this case
    // because the local maps direclty get there data
    // form the global map.
    auto now = simTime();
    if (lastUpdate >= now) {
      return;
    }
    lastUpdate = now;

    // global map: needs reset (not clear)
    dcdMapGlobal->visitCells(ResetVisitor{lastUpdate});
    dcdMapGlobal->clearNeighborhood();

    for(const auto& e: _table){
        const auto &posTraci = converter->position_cast_traci(e.second->getPos());
        dcdMapGlobal->incrementLocal(posTraci, e.second->getNodeId(), now);
    }

    // global map: local selector
    valueVisitor->setTime(simTime());
    dcdMapGlobal->computeValues(valueVisitor);

    // update decentralized map
    for (auto &handler : dezentralMaps) {
      handler.second->updateLocalMap();
      handler.second->computeValues();
    }
}

void GlobalEntropyMap::updateEntropy(){
    int numberOfEntries = 1000;
    auto now = simTime();
    auto prevUpdate = lastEntropyUpdate;
    if (now > lastEntropyUpdate){
        // todo make genric...
        for(int sourceId=0; sourceId < numberOfEntries; sourceId++){
            if (_table.find(sourceId) == _table.end()){
                BeaconReceptionInfo* info = new BeaconReceptionInfo();
                info->setNodeId(sourceId);
                take(info);
                _table[sourceId] = info;
            }
            // set some coordiante in the given bound
            _table[sourceId]->setPos({uniform(0.0, simBoundWidth), uniform(0.0, simBoundHeight)});
            // set some creation time stamp between last update and now
            _table[sourceId]->setReceivedTimePrio(uniform(prevUpdate.dbl(), now.dbl()));
        }
        // remove old values
        for( auto it=_table.cbegin(); it !=_table.cend();){
            if (it->second->getReceivedTimePrio() < prevUpdate){
                delete it->second;
                it = _table.erase(it);
            } else {
                ++it;
            }
        }
    }
    lastEntropyUpdate = now;
}


// iterator default to all elements in map
NeighborhoodTableIter_t GlobalEntropyMap::iter() {
    return IBaseNeighborhoodTable::all(&_table);
}

NeighborhoodTableIter_t GlobalEntropyMap::iter(NeighborhoodTablePred_t predicate){
    return NeighborhoodTableIter_t(&_table, predicate);
}

}
