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

void DensityMapAppSimple::updateLocalMap() { /* do nothing see NeighborhoodEntryListner */}


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
                dcdMap->getEntry<GridEntry>(oldCell)->decrementCount(simTime(), oldInfo->getBeaconValue());
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
        dcdMap->getEntry<GridEntry>(cellId)->incrementCount(simTime(), newInfo->getBeaconValue());
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
        dcdMap->getEntry<GridEntry>(oldCell)->decrementCount(simTime(), info->getBeaconValue());
        dcdMap->removeFromNeighborhood((int)info->getNodeId());
    }
}



} // namespace crownet
