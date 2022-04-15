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

GridCellID GridCellIDKeyProvider::getCellKey(const inet::Coord& pos){
    return getCellKey(converter->position_cast_traci(pos));
}

bool GridCellIDKeyProvider::changedCell(const traci::TraCIPosition& pos1, const traci::TraCIPosition& pos2){
    auto id1 = getCellKey(pos1);
    auto id2 = getCellKey(pos2);
    return id1 != id2;
}

bool GridCellIDKeyProvider::changedCell(const inet::Coord& pos1, const inet::Coord& pos2){
    return changedCell(converter->position_cast_traci(pos1), converter->position_cast_traci(pos2));
}


traci::TraCIPosition GridCellIDKeyProvider::getCellPosition(const traci::TraCIPosition& pos){
    return getCellPosition(getCellKey(pos));
}

traci::TraCIPosition GridCellIDKeyProvider::getCellPosition(const GridCellID& gridCell){
    return traci::TraCIPosition{gridCell.x()*converter->getCellSize().x, gridCell.y()*converter->getCellSize().y};
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
    entryDist.sourceHost = cellCenterDist(source, owner);
    entryDist.hostEntry = cellCenterDist(owner, entry);
    return entryDist;
}

EntryDist GridCellIDKeyProvider::getExactDist(const inet::Coord source, const inet::Coord owner, const GridCellID& entry){
    /*
     *  source, owner are opp coordinates (origin upper left) and GridId based cells are in traci coordinates (origin lower left).
     */
    inet::Coord oppEntryCenter = converter->moveCoordinateSystemTraCi_Opp(
            gridInfo.getCellCenter(entry.x(), entry.y())
            );

    EntryDist entryDist;
    entryDist.sourceEntry = source.distance(oppEntryCenter);
    entryDist.sourceHost = source.distance(owner);
    entryDist.hostEntry = owner.distance(oppEntryCenter);
    return entryDist;
}

EntryDist GridCellIDKeyProvider::getExactDist(const inet::Coord source, const inet::Coord owner, const GridCellID& entry, const double sourceEntry){
    /*
     *  source, owner are opp coordinates (origin upper left) and GridId based cells are in traci coordinates (origin lower left).
     */
    inet::Coord oppEntryCenter = converter->moveCoordinateSystemTraCi_Opp(
            gridInfo.getCellCenter(entry.x(), entry.y())
            );

    EntryDist entryDist;
    entryDist.sourceEntry = sourceEntry;
    entryDist.sourceHost = source.distance(owner);
    entryDist.hostEntry = owner.distance(oppEntryCenter);
    return entryDist;
}

}  // namespace crownet
