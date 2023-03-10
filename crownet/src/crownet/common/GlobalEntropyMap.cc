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

#include "crownet/common/GlobalEntropyMap.h"
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
      setLastUpdatedAt(-1.0);
      if (entropyTimerInterval > 0) {
            scheduleAt(simTime(), entropyTimer); // schedule now
      }

      entropyProvider = (EntropyProvider*)(par("entropyProvider").objectValue()->dup());
      take(entropyProvider);
      entropyProvider->initialize(getRNG(par("entropyRngGenerator").intValue()));
      mapDataType = "entropyData";
  }
}

void GlobalEntropyMap::visitNode(const std::string& traciNodeId, omnetpp::cModule* mod) {
  const auto mobility = check_and_cast<inet::IMobility*>(mod->getModuleByPath(m_mobilityModule.c_str()));
  // convert to traci 2D position
  const auto &pos = mobility->getCurrentPosition();
  const auto &posTraci = converter->position_cast_traci(pos);
  const int cellid = -1*cellKeyProvider->getCellKey1D(posTraci);
  if (_table.find(cellid) == _table.end()){
      // value for cell not calculated jet. Call getValue to set the value
      // base on the last update time.
      getValue(cellid);
  }

  // visitNode is called for *all* nodes thus this 'local' map of the global
  // module represents the global (ground truth) of the simulation.
  // only update position, rest will be done in the update method
  auto e = dcdMapGlobal->getEntry<GridGlobalEntry>(posTraci);
  e->nodeIds.insert(mod->getId());
  e->touch(simTime()); // ensure entry is valid.
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
    // log agent cell position. Will update _table with missing entires
    acceptNodeVisitor(this);

    for(const auto& e: _table){
        const auto currentData = e.second->getCurrentData();
        const auto &posTraci = converter->position_cast_traci(currentData->getPosition());
        // IMPORTANT: Assume additive value. GlobalEntropyMap produces ONE info object
        //            for each cell so setValue will write once.
        auto ee = dcdMapGlobal->getEntry<GridGlobalEntry>(posTraci);
        ee->setValue(now, currentData->getBeaconValue());
        // do not add nodeId because the ID is a dummy from the Entropy provider.
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

NeighborhoodTableValue_t GlobalEntropyMap::getValue(const int sourceId){
    auto lastUpdateTime = getLastUpdatedAt();
    if (_table.find(sourceId) == _table.end()){
        // no value for cell found. Create new entry and set defaults
        BeaconReceptionInfo* info = new BeaconReceptionInfo();
        info->initAppData();
        info->setNodeId(sourceId);
        info->getCurrentDataForUpdate()->setReceivedTime(-1.0); // ensure time is before 'time'
        auto cellCenter = cellKeyProvider->cellCenter(sourceId);
        info->getCurrentDataForUpdate()->setPosition(cellCenter);
        take(info);
        _table[sourceId] = info;
    }
    auto ret = _table[sourceId];
    if (ret->getCurrentData()->getReceivedTime() < lastUpdateTime){
        // if value ret not updated at lastUpdateTime perform update at lastUpdateTime.
        // This allows lazy update of ground truth when the value is requested. The
        // current time is not needed because lastUpdateTime will be set by the event trigger.
        auto pos = ret->getCurrentData()->getPosition();
        ret->getCurrentDataForUpdate()->setBeaconValue(entropyProvider->getValue(pos, lastUpdateTime, ret->getCurrentData()->getBeaconValue()));
        ret->getCurrentDataForUpdate()->setReceivedTime(lastUpdateTime);
    }
    return *_table.find(sourceId);
}

NeighborhoodTableValue_t GlobalEntropyMap::getValue(const GridCellID& cellId){
    auto id = -1*cellKeyProvider->getCellKey1D(cellId);
    return getValue(id);
}
NeighborhoodTableValue_t GlobalEntropyMap::getValue(const inet::Coord& pos){
    auto id = -1*cellKeyProvider->getCellKey1D(pos);
    return getValue(id);
}

void GlobalEntropyMap::updateEntropy(){
    auto now = simTime();
    auto prevUpdate = getLastUpdatedAt();
    if (now > prevUpdate){
        setLastUpdatedAt(now); //set lastUpdated time to current time.
        for(const auto& entry: _table){
            //only update values already entered once
            getValue(entry.first);
        }
    }
    EV_INFO << LOG_MOD2 << _table.size() << " entries in Entropy table." << endl;

}


// iterator default to all elements in map
NeighborhoodTableIter_t GlobalEntropyMap::iter() {
    return NeighborhoodTableLazyIter_t(&_table, IBaseNeighborhoodTable::all_pred(), this);
}

NeighborhoodTableIter_t GlobalEntropyMap::iter(NeighborhoodTablePred_t predicate){
    return NeighborhoodTableLazyIter_t(&_table, predicate, this);
}

}
