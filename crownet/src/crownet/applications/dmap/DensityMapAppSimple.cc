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

#include "crownet/applications/dmap/DensityMapAppSimple.h"

#include "crownet/crownet.h"

namespace crownet {

Define_Module(DensityMapAppSimple);

void DensityMapAppSimple::finish(){
    nTable->removeEntryListener(this);
}

void DensityMapAppSimple::initialize(int stage) {
    BaseDensityMapApp::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        nTable = inet::getModuleFromPar<INeighborhoodTable>(par("neighborhoodTableMobdule"), inet::getContainingNode(this));
        nTable->setOwnerId(hostId);
        nTable->registerEntryListner(this);
    }
}

void DensityMapAppSimple::computeValues() {
    nTable->checkTimeToLive();
    BaseDensityMapApp::computeValues();
}

void DensityMapAppSimple::updateLocalMap() {
//  simtime_t now = simTime();
//  if (lastUpdate >= simTime()) {
//    return;
//  }
//  lastUpdate = now;
//  EV_INFO << LOG_MOD << "Update local map[" << dcdMap->getOwnerId() <<"]:" << endl;
//  if (hostId == 231){
//      std::cout << "" << endl;
//  }
//
//  // set count of all cells in local map to zero.
//  // do not change the valid state.
//  clearLocal->setTime(simTime());
////  dcdMap->visitCells(ClearLocalVisitor{simTime()});
//  dcdMap->visitCells(clearLocal);
//  dcdMap->clearNeighborhood();
//
//  // update own cell
//  dcdMap->setOwnerCell(converter->position_cast_traci(getPosition()));
//
//  // update neighborhood table before access.
//  int nTableCount = 0;
//  nTable->checkTimeToLive();
//  auto iter = nTable->iter();
//  for (const auto& entry: iter){
//      int _id = entry.first;
//      auto info = entry.second;
//      auto pos = converter->position_cast_traci(info->getPos());
//      dcdMap->incrementLocal(
//              pos,
//              _id,
//              now, // use time of measurement not time of beacon creation
//              info->getBeaconValue());
//      ++nTableCount;
//      auto d =  info->getPos().distance(getPosition());
//      if (hostId == 231){
//          std::cout << LOG_MOD2 << "   " << nTableCount << " id: " << _id << "  pos: " << info->getPos() << " dist: "  << d << endl;
//      }
//      GridCellID id{1,1};
//      auto foo = dcdMap->getEntry<GridLocalEntry>(id);
//
//  }
////  std::cout << LOG_MOD2 << "Found " << nTableCount << " entries in neighborhood table" << endl;
////  std::cout << LOG_MOD2 << dcdMap->str() << endl;
//  if (hostId == 231){
//      std::cout << "" << endl;
//  }
//
//  using namespace omnetpp;
//  EV_DEBUG << dcdMap->strFull() << std::endl;
}


void DensityMapAppSimple::neighborhoodEntryPreChanged(INeighborhoodTable* table, BeaconReceptionInfo* oldInfo) {
    // Note: Implementation assumes additive cell entry values. Each beacon provides an additive portion of the
    //       cell value. Used for node counts.
    //       Workaround for absolute values (i.e. temperature, entropy values): Ensure at most ONE beacon source (nodeId)
    //       for each cell. Then the additive logic with increment/decrement will also work.
    //
    // decrement count in old entry. Do not remove source from neighborhood. This will be done in the PostChange listener.
    if (isRunning()){
        if (dcdMap->isInNeighborhood((int)oldInfo->getNodeId())){
            EV_TRACE << LOG_MOD << hostId << " preChange:" << cObjectPrinter::shortBeaconInfoShortPrinter(oldInfo) << endl;
            auto oldCell = dcdMap->getNeighborCell((int)oldInfo->getNodeId());
            if (dcdMap->hasEntry(oldCell)){
                // if DcdMap contains a (local) entry for the cell decrement.
                dcdMap->getEntry<GridLocalEntry>(oldCell)->decrementCount(simTime(), oldInfo->getBeaconValue());
            }
        }
    }
}


void DensityMapAppSimple::neighborhoodEntryPostChanged(INeighborhoodTable* table, BeaconReceptionInfo* newInfo){
    // Note: Implementation assumes additive cell entry values. Each beacon provides an additive portion of the
    //       cell value. Used for node counts.
    //       Workaround for absolute values (i.e. temperature, entropy values): Ensure at most ONE beacon source (nodeId)
    //       for each cell. Then the additive logic with increment/decrement will also work.
    //
    // increment/update entry based on new beacon informationgetCellId
    if (isRunning()){
        EV_TRACE << LOG_MOD << hostId << " postChange:" << cObjectPrinter::shortBeaconInfoShortPrinter(newInfo) << endl;
        auto pos = converter->position_cast_traci(newInfo->getPos());
        auto cellId = dcdMap->getCellId(pos);
        dcdMap->getEntry<GridLocalEntry>(cellId)->incrementCount(simTime(), newInfo->getBeaconValue());
        // update position of beacon source in neighborhood table.
        dcdMap->addToNeighborhood((int)newInfo->getNodeId(), pos);
        if (dcdMap->getOwnerId().value() == (int)newInfo->getNodeId()){
            // update owner location if beacon comes from owner.
            dcdMap->setOwnerCell(cellId);
        }
    }
}

void DensityMapAppSimple::neighborhoodEntryRemoved(INeighborhoodTable* table, BeaconReceptionInfo* info){
    // Note: Implementation assumes additive cell entry values. Each beacon provides an additive portion of the
    //       cell value. Used for node counts.
    //       Workaround for absolute values (i.e. temperature, entropy values): Ensure at most ONE beacon source (nodeId)
    //       for each cell. Then the additive logic with increment/decrement will also work.
    //
    // remove beacon value from cell entry and remove source (nodeId) from neighborhood
    if (isRunning()){
        EV_TRACE << LOG_MOD << hostId << " remove:" << cObjectPrinter::shortBeaconInfoShortPrinter(info) << endl;
        auto oldCell = dcdMap->getNeighborCell((int)info->getNodeId());
        dcdMap->getEntry<GridLocalEntry>(oldCell)->decrementCount(simTime(), info->getBeaconValue());
        dcdMap->removeFromNeighborhood((int)info->getNodeId());
    }
}



} // namespace crownet
