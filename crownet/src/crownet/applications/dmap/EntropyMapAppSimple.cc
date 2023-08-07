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
        entropyClient = inet::getModuleFromPar<EntropyNeigborhoodTableClient>(par("neighborhoodTableModule"), inet::getContainingNode(this));
        entropyClient->setOwnerId(hostId);
        mapDataType = DpmmMapType::ENTROPY;
    }
}

void EntropyMapAppSimple::applyContentTags(Ptr<Chunk> content){
    BaseApp::applyContentTags(content);
    // mark packet as Entropy Packet and not pedestrian count
    content->addTagIfAbsent<EntropyMap>();
}

std::string  EntropyMapAppSimple::getMapName() const{
    std::stringstream s;
    s << "entropyMap_" << hostId;
    return s.str();
}

void EntropyMapAppSimple::updateLocalMap() {
    /* access entropyClient (nTable) and load data into map */
    auto now = simTime();
    if (lastEntropyUpdate >= now){
        return;
    }
    lastEntropyUpdate = now;
    // do not clear 'old' information. The information will be updated
    // only for the cells that are reachable. Older values will stay in the map
    // until the map TTL removes them or the agent enters the cell again.

    // access entropy client based on distance configuration.
    int count = 0;
    if (entropyClient->currentCellOnly()){
        // only 'sense' current cell
        count = 1;
        const auto &posTraci = converter->position_cast_traci(getPosition());
        const auto info = entropyClient->updateGetGlobalValue(getPosition());
        auto ee = dcdMap->getEntry<GridEntry>(posTraci);
        ee->setValue(now, info.second->getCurrentData()->getBeaconValue());

    } else {
        // interator has filters for distance
        for(const auto& e : entropyClient->iter()){
            ++count;
            const auto info = e.second;
            const auto &posTraci = converter->position_cast_traci(info->getCurrentData()->getPosition());
                    // IMPORTANT: Assume additive value. GlobalEntropyMap produces ONE info object
                    //            for each cell so the additive setup works here!
            auto ee = dcdMap->getEntry<GridEntry>(posTraci);
            ee->setValue(now, info->getCurrentData()->getBeaconValue()); // entropy sever ensures that ther is at most on
                                                             // entry per cell. Thus increment works here.
         }
    }

//     std::cout << LOG_MOD2 << count << " entries in Entropy table." << endl;

}
} // namespace crownet
