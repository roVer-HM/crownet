/*
 * CellKeyProvider.cc
 *
 *  Created on: Nov 30, 2020
 *      Author: sts
 */

#include "crownet/dcd/identifier/CellKeyProvider.h"

#include <math.h>

namespace crownet {

const GridCellID GridCellIDKeyProvider::getCellKey(const traci::TraCIPosition& pos) const{
    return gridInfo.getCellKey(pos);
}

const GridCellID GridCellIDKeyProvider::getCellKey(const inet::Coord& pos) const{
    return getCellKey(converter->position_cast_traci(pos));
}
const GridCellID GridCellIDKeyProvider::getCellKey(const int cellKey1D) const {
    return gridInfo.getCellKey(cellKey1D);
}

const int GridCellIDKeyProvider::getCellKey1D(const traci::TraCIPosition& pos) const{
    return gridInfo.getCellKey1D(pos);
}
const int GridCellIDKeyProvider::getCellKey1D(const inet::Coord& pos) const{
    return getCellKey1D(converter->position_cast_traci(pos));
}
const int GridCellIDKeyProvider::getCellKey1D(const GridCellID& cellKey) const {
    return gridInfo.getCellKey1D(cellKey);
}


bool GridCellIDKeyProvider::changedCell(const traci::TraCIPosition& pos1, const traci::TraCIPosition& pos2){
    auto id1 = getCellKey(pos1);
    auto id2 = getCellKey(pos2);
    return id1 != id2;
}

bool GridCellIDKeyProvider::changedCell(const inet::Coord& pos1, const inet::Coord& pos2){
    return changedCell(converter->position_cast_traci(pos1), converter->position_cast_traci(pos2));
}


const traci::TraCIPosition GridCellIDKeyProvider::getCellPosition(const traci::TraCIPosition& pos){
    return getCellPosition(getCellKey(pos));
}

const traci::TraCIPosition GridCellIDKeyProvider::getCellPosition(const GridCellID& gridCell){
    return traci::TraCIPosition{gridCell.x()*converter->getCellSize().x, gridCell.y()*converter->getCellSize().y};
}

const inet::Coord GridCellIDKeyProvider::cellCenter(const traci::TraCIPosition& pos) const{
    return cellCenter(getCellKey(pos));
}
const inet::Coord GridCellIDKeyProvider::cellCenter(const inet::Coord& pos) const{
    return cellCenter(getCellKey(pos));
}
const inet::Coord GridCellIDKeyProvider::cellCenter(const GridCellID& cellKey) const{
    auto traciPos =  gridInfo.getCellCenter(cellKey);
    return converter->position_cast_inet(traciPos);
}
const inet::Coord GridCellIDKeyProvider::cellCenter(const int cellKey1D) const{
    auto traciPos = gridInfo.getCellCenter(getCellKey(cellKey1D));
    return converter->position_cast_inet(traciPos);
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
    inet::Coord oppEntryCenter = cellCenter(entry);

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
    inet::Coord oppEntryCenter = cellCenter(entry);

    EntryDist entryDist;
    entryDist.sourceEntry = sourceEntry;
    entryDist.sourceHost = source.distance(owner);
    entryDist.hostEntry = owner.distance(oppEntryCenter);
    return entryDist;
}

}  // namespace crownet
