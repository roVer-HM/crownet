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
    BaseDensityMapApp::finish();
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
    nTable->checkAllTimeToLive();
    BaseDensityMapApp::computeValues();
}

void DensityMapAppSimple::updateLocalMap() {
    /*
     * do nothing here. Local map is updated using
     * NeighborhoodEntryListners below
     */
}

//FSM
FsmState DensityMapAppSimple::fsmSetup(cMessage *msg) {
    auto ret = BaseDensityMapApp::fsmSetup(msg);
    initLocalMap();
    return ret;
}

/**
 * Read already existing neighborhood into density map.
 */
void DensityMapAppSimple::initLocalMap(){

    nTable->checkAllTimeToLive();
    for (const auto& el : nTable->iter()){
        neighborhoodEntryEnterCell(nTable, el.second);
    }

}


void DensityMapAppSimple::neighborhoodEntryRemoved(INeighborhoodTable* table, BeaconReceptionInfo* info){
    // Note: Implementation assumes additive cell entry values. Each beacon provides an additive portion of the
    //       cell value. Used for node counts.
    //       Workaround for absolute values (i.e. temperature, entropy values): Ensure at most ONE beacon source (nodeId)
    //       for each cell. Then the additive logic with increment/decrement will also work.
    //
    // remove beacon value from cell entry and remove source (nodeId) from neighborhood
    Enter_Method_Silent();
    if (isRunning()){
        EV_INFO << LOG_MOD << hostId << " remove:" << cObjectPrinter::shortBeaconInfoShortPrinter(info) << endl;
        auto cellId = dcdMap->getNeighborCell((int)info->getNodeId());
        auto cellEntryLocal = dcdMap->getEntry<GridEntry>(cellId);
        // TTL reached thus use the current simulation time and not the
        // time stamps present in the info object for both values.
        cellEntryLocal->decrementCount(
                simTime(),
                1.0
        );
        dcdMap->removeFromNeighborhood((int)info->getNodeId());
    }
}

void DensityMapAppSimple::neighborhoodEntryLeaveCell(INeighborhoodTable* table, BeaconReceptionInfo* info){
/*
 * Node moved from one cell to another. Therefore the old cell must be decrement using the new sent time
 * as the change time. The info object must contain the current and prio value. If not this method will
 * raise a runtime error.
 *
 */
    Enter_Method_Silent();
    if (isRunning()){
        if (!info->hasPrio()){
            throw cRuntimeError("Beacon info object does not contain prio values");
        }
        // get and check prio position
        auto cellId = dcdMap->getCellKeyProvider()->getCellKey(info->getPositionPrio());
        auto savedCellId = dcdMap->getNeighborCell((int)info->getNodeId());
        if (cellId != savedCellId){
            //todo add info to error message
            throw cRuntimeError("BeaconReceptionInfo reports different cell id than saved in density map");
        }
        EV_INFO << LOG_MOD << hostId << " leave-cell:" << cObjectPrinter::shortBeaconInfoShortPrinter(info) << endl;
        // create distance measure
        auto dist = dcdMap->getCellKeyProvider()->getExactDist(
                getPosition(),  // source
                getPosition(),  // owner
                cellId  // cell
        );

        // decrement old cell by 1, update time and distance measure
        auto cellEntryLocal = dcdMap->getEntry<GridEntry>(cellId);
        cellEntryLocal->decrementCount(
                info->getSentSimTimeCurrent(),
                info->getReceivedTimeCurrent(),
                1.0
        );
        cellEntryLocal->setEntryDist(std::move(dist));
        // remove node from Map NT
        dcdMap->removeFromNeighborhood((int)info->getNodeId());
    }
}

void DensityMapAppSimple::neighborhoodEntryEnterCell(INeighborhoodTable* table, BeaconReceptionInfo* info){
/*
 * Node moved into a cell or this is the first beacon receivd from this node. The info object my contain
 * data from a prio position but this is not needed here. This method only uses the current data.
 */
    Enter_Method_Silent();
    if (isRunning()){
        // get and check current position
        auto cellId = dcdMap->getCellKeyProvider()->getCellKey(info->getPositionCurrent());
        if (dcdMap->isInNeighborhood((int)info->getNodeId())){
            //todo add info to error message
            throw cRuntimeError("Node already in neighborhood");
        }
        EV_INFO << LOG_MOD << hostId << " enter-cell:" << cObjectPrinter::shortBeaconInfoShortPrinter(info) << endl;
        // create distance measure
        auto dist = dcdMap->getCellKeyProvider()->getExactDist(
                getPosition(),  // source
                getPosition(),  // owner
                cellId  // cell
        );

        // increment new cell by 1, update time and distance measure
        auto cellEntryLocal = dcdMap->getEntry<GridEntry>(cellId);
        cellEntryLocal->incrementCount(
                info->getSentSimTimeCurrent(),
                info->getReceivedTimeCurrent(),
                1.0
        );
        cellEntryLocal->setEntryDist(std::move(dist));
        // add node to neighborhood
        dcdMap->addToNeighborhood((int)info->getNodeId(), cellId);
    }

}

void DensityMapAppSimple::neighborhoodEntryStayInCell(INeighborhoodTable* table, BeaconReceptionInfo* info){
/*
 * Node stayed in the same cell compared to the prio beacon received. The position may differ but it does not
 * have to. The info object must contain the current and prio value. The position of the prio and current value
 * may differ but must map to the SAME cell. This method will not change the count of this cell and will only
 * update the time stamps of the measurement.
 */
    Enter_Method_Silent();
    if (isRunning()){
        if (!info->hasPrio()){
            throw cRuntimeError("Beacon info object does not contain prio values");
        }
        // get and check position for cellId
        auto cellId_prio = dcdMap->getCellKeyProvider()->getCellKey(info->getPositionPrio());
        auto cellId_current = dcdMap->getCellKeyProvider()->getCellKey(info->getPositionCurrent());
        auto savedCellId = dcdMap->getNeighborCell((int)info->getNodeId());
        if ((cellId_prio != cellId_current) || (cellId_current !=savedCellId)){
            //todo add info to error message
            throw cRuntimeError("Nodes prio and current cell do not match or current position "
                    "and saved cell in map do not match");
        }
        EV_INFO << LOG_MOD << hostId << " stay-in-cell:" << cObjectPrinter::shortBeaconInfoShortPrinter(info) << endl;
        // create distance measure
        auto dist = dcdMap->getCellKeyProvider()->getExactDist(
                getPosition(),  // source
                getPosition(),  // owner
                cellId_current  // cell
        );

        // update time stamps and distance
        auto cellEntryLocal = dcdMap->getEntry<GridEntry>(cellId_current);
        cellEntryLocal->touch(
            info->getSentSimTimeCurrent(),
            info->getReceivedTimeCurrent()
        );
        cellEntryLocal->setEntryDist(std::move(dist));
    }
}



} // namespace crownet
