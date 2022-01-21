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

#include "crownet/applications/dmap/EntropyMapAppSimple.h"
#include "crownet/crownet.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"


namespace crownet {

Define_Module(EntropyMapAppSimple);

void EntropyMapAppSimple::initialize(int stage) {
    BaseDensityMapApp::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        entropyClient = inet::getModuleFromPar<EntropyNeigborhoodTableClient>(par("neighborhoodTableMobdule"), inet::getContainingNode(this));
        entropyClient->setOwnerId(hostId);
    }
}

void EntropyMapAppSimple::applyContentTags(Ptr<Chunk> content){
    BaseApp::applyContentTags(content);
    // mark packet as Entropy Packet and not pedestrian count
    content->addTagIfAbsent<EntropyMap>();
}

void EntropyMapAppSimple::updateLocalMap() {
    // todo move to other function and call before sending data
    // check entropy server for last change to only update if
    // data changed!.

    /* access entropyClient (nTable) and load data into map */
    auto now = simTime();
    if (lastEntropyUpdate >= now){
        return;
    }
    lastEntropyUpdate = now;
    /**
     * For entropy based maps the whole 'local' view is read at once. Thus the old
     * state is reset.
     */
    dcdMap->visitCells(ResetLocalVisitor{lastEntropyUpdate});
    dcdMap->clearNeighborhood(); // not used for entrophy data.

    // access entropy client based on distance configuration.
    int count = 0;
    for(const auto& e : entropyClient->iter()){
        ++count;
        const auto info = e.second;
        const auto &posTraci = converter->position_cast_traci(info->getPos());
                // IMPORTANT: Assume additive value. GlobalEntropyMap produces ONE info object
                //            for each cell so the additive setup works here!
        auto ee = dcdMap->getEntry<GridEntry>(posTraci);
        ee->incrementCount(now, info->getBeaconValue()); // entropy sever ensures that ther is at most on
                                                         // entry per cell. Thus increment works here.
     }
//     std::cout << LOG_MOD2 << count << " entries in Entropy table." << endl;

}


//void DensityMapAppSimple::neighborhoodEntryPreChanged(INeighborhoodTable* table, BeaconReceptionInfo* oldInfo) {
//    // Note: Implementation assumes additive cell entry values. Each beacon provides an additive portion of the
//    //       cell value. Used for node counts.
//    //       Workaround for absolute values (i.e. temperature, entropy values): Ensure at most ONE beacon source (nodeId)
//    //       for each cell. Then the additive logic with increment/decrement will also work.
//    //
//    // decrement count in old entry. Do not remove source from neighborhood. This will be done in the PostChange listener.
//    if (isRunning()){
//        if (dcdMap->isInNeighborhood((int)oldInfo->getNodeId())){
//            EV_INFO << LOG_MOD << hostId << " preChange:" << cObjectPrinter::shortBeaconInfoShortPrinter(oldInfo) << endl;
//            auto oldCell = dcdMap->getNeighborCell((int)oldInfo->getNodeId());
//            if (dcdMap->hasEntry(oldCell)){
//                // if DcdMap contains a (local) entry for the cell decrement.
//                dcdMap->getEntry<GridEntry>(oldCell)->decrementCount(simTime(), oldInfo->getBeaconValue());
//            }
//        }
//    }
//}
//
//
//void DensityMapAppSimple::neighborhoodEntryPostChanged(INeighborhoodTable* table, BeaconReceptionInfo* newInfo){
//    // Note: Implementation assumes additive cell entry values. Each beacon provides an additive portion of the
//    //       cell value. Used for node counts.
//    //       Workaround for absolute values (i.e. temperature, entropy values): Ensure at most ONE beacon source (nodeId)
//    //       for each cell. Then the additive logic with increment/decrement will also work.
//    //
//    // increment/update entry based on new beacon informationgetCellId
//    if (isRunning()){
//        EV_INFO << LOG_MOD << hostId << " postChange:" << cObjectPrinter::shortBeaconInfoShortPrinter(newInfo) << endl;
//        // update own position
//        updateOwnLocationInMap();
//        auto ownerCellId = dcdMap->getOwnerCell();
//
//        // access or create cell entry containing the beacon just received and increment the count.
//        auto beaconSourcePos = converter->position_cast_traci(newInfo->getPos());
//        auto beaconCellId = dcdMap->getCellId(beaconSourcePos);
//        auto cellEntryLocal = dcdMap->getEntry<GridEntry>(beaconCellId);
//        cellEntryLocal->incrementCount(simTime(), newInfo->getBeaconValue());
//        // update the entry distance struct. Current node (ownerCell is both the source and owner in the entry distance struct.
//        // because this node 'measures' the  value in the cell from which the beacon comes from.
//        cellEntryLocal->setEntryDist(dcdMap->getCellKeyProvider()->getEntryDist(ownerCellId, ownerCellId, beaconCellId));
//
//        // update position of beacon source in neighborhood table.
//        dcdMap->addToNeighborhood((int)newInfo->getNodeId(), beaconSourcePos);
//        if (dcdMap->getOwnerId().value() == (int)newInfo->getNodeId()){
//            // update owner location if beacon comes from owner.
//            dcdMap->setOwnerCell(beaconCellId);
//        }
//    }
//}
//
//void DensityMapAppSimple::neighborhoodEntryRemoved(INeighborhoodTable* table, BeaconReceptionInfo* info){
//    // Note: Implementation assumes additive cell entry values. Each beacon provides an additive portion of the
//    //       cell value. Used for node counts.
//    //       Workaround for absolute values (i.e. temperature, entropy values): Ensure at most ONE beacon source (nodeId)
//    //       for each cell. Then the additive logic with increment/decrement will also work.
//    //
//    // remove beacon value from cell entry and remove source (nodeId) from neighborhood
//    if (isRunning()){
//        EV_INFO << LOG_MOD << hostId << " remove:" << cObjectPrinter::shortBeaconInfoShortPrinter(info) << endl;
//        auto oldCell = dcdMap->getNeighborCell((int)info->getNodeId());
//        dcdMap->getEntry<GridEntry>(oldCell)->decrementCount(simTime(), info->getBeaconValue());
//        dcdMap->removeFromNeighborhood((int)info->getNodeId());
//    }
//}



} // namespace crownet
