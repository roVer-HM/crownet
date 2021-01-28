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

#include "DensityMapAppSimple.h"

#include "crownet/dcd/regularGrid/RegularCellVisitors.h"

namespace crownet {

Define_Module(DensityMapAppSimple);

void DensityMapAppSimple::initialize(int stage) {
    BaseDensityMapApp::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mobility = inet::getModuleFromPar<inet::IMobility>(par("mobilityModule"), inet::getContainingNode(this));
        nTable = inet::getModuleFromPar<NeighborhoodTable>(par("neighborhoodTableMobdule"), inet::getContainingNode(this));
    }
}

void DensityMapAppSimple::updateLocalMap() {
  simtime_t measureTime = simTime();
  if (lastUpdate >= simTime()) {
    return;
  }
  lastUpdate = measureTime;

  // set count of all cells in local map to zero.
  // do not change the valid state.
  dcdMap->visitCells(ClearLocalVisitor{simTime()});
  dcdMap->clearNeighborhood();

  // add yourself to the map.
  const auto &pos = mobility->getCurrentPosition();
  const auto &posInet = converter->position_cast_traci(pos);

  dcdMap->setOwnerCell(posInet);
  dcdMap->incrementLocal(posInet, dcdMap->getOwnerId(), measureTime);

  for (const auto& entry: *nTable){
      int _id = entry.first;
      auto pos = converter->position_cast_traci(entry.second.getPos());
      // increment density map
//      dcdMap->incrementLocal(pos, _id, entry.second.getTimeReceived());
      dcdMap->incrementLocal(pos, _id, measureTime);
  }

  using namespace omnetpp;
  EV_DEBUG << dcdMap->strFull() << std::endl;
}


} // namespace crownet
