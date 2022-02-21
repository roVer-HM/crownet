/*
 * CellKeyProvider.cc
 *
 *  Created on: Nov 30, 2020
 *      Author: sts
 */

#include "crownet/dcd/identifier/CellKeyProvider.h"

#include <math.h>

namespace crownet {

GridCellID GridCellIDKeyProvider::getCellKey(const traci::TraCIPosition& pos) {
  int x_id = floor(pos.x / getCellSize().x);
  int y_id = floor(pos.y / getCellSize().y);
  return GridCellID(x_id, y_id);
}

traci::TraCIPosition GridCellIDKeyProvider::getCellPosition(const traci::TraCIPosition& pos){
    return getCellPosition(getCellKey(pos));
}

traci::TraCIPosition GridCellIDKeyProvider::getCellPosition(const GridCellID& gridCell){
    return traci::TraCIPosition{gridCell.x()*gridInfo.getCellSize().x, gridCell.y()*gridInfo.getCellSize().y};
}


double GridCellIDKeyProvider::cellCenterDist(const GridCellID& cell1, const GridCellID&  cell2) {
    if (cell1 == cell2){
        return 0.0;
    } else if (cell2 < cell1){
        return this->cellCenterDist(cell2, cell1);
    }
    // cell1 < cell2 after here.
    const auto p = std::make_pair(cell1, cell2);
    if (distMap.find(p) == distMap.end()){
        distMap[p] = gridInfo.cellCenterDist(cell1, cell2);
    }
    return distMap[p];
}

EntryDist GridCellIDKeyProvider::getEntryDist(const GridCellID& source, const GridCellID& owner, const GridCellID& entry) {
    EntryDist entryDist;
    entryDist.sourceEntry = cellCenterDist(source, entry);
    entryDist.sourceOwner = cellCenterDist(source, owner);
    entryDist.ownerEntry = cellCenterDist(owner, entry);
    return entryDist;
}

}  // namespace crownet
