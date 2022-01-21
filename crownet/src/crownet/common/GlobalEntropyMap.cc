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
#include "crownet/crownet.h"


using namespace omnetpp;
using namespace inet;

namespace crownet {

Define_Module(GlobalEntropyMap);


GlobalEntropyMap::~GlobalEntropyMap(){
    if (entropyTimer) cancelAndDelete(entropyTimer);
    delete entropyProvider;
}


void GlobalEntropyMap::initialize(int stage) {
    GlobalDensityMap::initialize(stage);
  if (stage == INITSTAGE_LOCAL) {
      // setup entropy
      entropyTimer = new cMessage("entropyInterval");
      entropyTimerInterval = par("entropyInterval").doubleValue();
      if (entropyTimerInterval > 0) {
            scheduleAt(simTime() + 1.0, entropyTimer);
      }

      entropyProvider = (EntropyProvider*)(par("entropyProvider").objectValue()->dup());
      take(entropyProvider);
      entropyProvider->initialize(getRNG(par("entropyRngGenerator").intValue()));
  }
}

void GlobalEntropyMap::handleMessage(cMessage *msg){
    if (msg == entropyTimer){
        // update nTable
        updateEntropy();

        // 3) reschedule
           scheduleAt(simTime() + entropyTimerInterval, entropyTimer);
    } else {
        GlobalDensityMap::handleMessage(msg);
    }
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
    // The reset is correct for the global view because the new state is
    // loaded completely!
    dcdMapGlobal->visitCells(ResetVisitor{lastUpdate});
    dcdMapGlobal->clearNeighborhood();

    for(const auto& e: _table){
        const auto info = e.second;
        const auto &posTraci = converter->position_cast_traci(info->getPos());
        // IMPORTANT: Assume additive value. GlobalEntropyMap produces ONE info object
        //            for each cell so the additive setup works here!
        auto ee = dcdMapGlobal->getEntry<GridGlobalEntry>(posTraci);
        ee->incrementCount(simTime(), info->getBeaconValue()); // increment by value.
        ee->nodeIds.insert((int)info->getNodeId());
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
    auto now = simTime();
    auto prevUpdate = getLastUpdatedAt();
    if (now > prevUpdate){
        for(int x = 0; x < grid.getCellCount().x ; x++) {
            for(int y = 0; y < grid.getCellCount().y; y++){
                if (entropyProvider->selectCell(x, y, now)){
                    auto pos = grid.getCellCenter(x, y);
                    double value = entropyProvider->getValue(pos, now);
                    if (value > 0.0){
                        // dummy id based on current cell.
                        int sourceId = -1*grid.getCellId(x, y);

                        // reuse object if present.
                        if (_table.find(sourceId) == _table.end()){
                            BeaconReceptionInfo* info = new BeaconReceptionInfo();
                            info->setNodeId(sourceId);
                            take(info);
                            _table[sourceId] = info;
                        }
                        _table[sourceId]->setPos(pos);
                        _table[sourceId]->setReceivedTimePrio(now);
                        _table[sourceId]->setBeaconValue(value);
                    }
                }
            }
        }
        // remove old values
        for( auto it=_table.cbegin(); it !=_table.cend();){
            if (it->second->getReceivedTimePrio() < now){
                delete it->second;
                it = _table.erase(it);
            } else {
                ++it;
            }
        }
    }
    std::cout << LOG_MOD2 << _table.size() << " entries in Entropy table." << endl;
    setLastUpdatedAt(now);
}


// iterator default to all elements in map
NeighborhoodTableIter_t GlobalEntropyMap::iter() {
    return IBaseNeighborhoodTable::all(&_table);
}

NeighborhoodTableIter_t GlobalEntropyMap::iter(NeighborhoodTablePred_t predicate){
    return NeighborhoodTableIter_t(&_table, predicate);
}

}
