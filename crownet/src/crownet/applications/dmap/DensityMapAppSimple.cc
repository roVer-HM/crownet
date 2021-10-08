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

#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/crownet.h"

namespace crownet {

Define_Module(DensityMapAppSimple);

void DensityMapAppSimple::initialize(int stage) {
    BaseDensityMapApp::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        nTable = inet::getModuleFromPar<INeighborhoodTable>(par("neighborhoodTableMobdule"), inet::getContainingNode(this));
        nTable->setOwnerId(hostId);
    }
}

void DensityMapAppSimple::updateLocalMap() {
  simtime_t now = simTime();
  if (lastUpdate >= simTime()) {
    return;
  }
  lastUpdate = now;
  EV_INFO << LOG_MOD << "Update local map[" << dcdMap->getOwnerId() <<"]:" << endl;

  // set count of all cells in local map to zero.
  // do not change the valid state.
  dcdMap->visitCells(ClearLocalVisitor{simTime()});
  dcdMap->clearNeighborhood();

  // update neighborhood table before access.
  int nTableCount = 0;
  nTable->checkTimeToLive();
  auto iter = nTable->iter();
  for (const auto& entry: iter){
      int _id = entry.first;
      auto info = entry.second;
      auto pos = converter->position_cast_traci(info->getPos());
      dcdMap->incrementLocal(
              pos,
              _id,
              now, // use time of measurement not time of beacon creation
              info->getBeaconValue());
      ++nTableCount;
  }
  EV_INFO << LOG_MOD2 << "Found " << nTableCount << " entries in neighborhood table" << endl;
  EV_INFO << LOG_MOD2 << dcdMap->str();

  using namespace omnetpp;
  EV_DEBUG << dcdMap->strFull() << std::endl;
}


} // namespace crownet
