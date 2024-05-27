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


bool GridCellIDKeyProvider::intersectLineCircle(const inet::Coord& a, const inet::Coord&  b, const double radiusSquared, const inet::Coord& center) const {
    // Find perpendicular foot from circle center to line,
    //  * ensure that the distance is smaller/equal to the radius and,
    //  * ensure that the perpendicular foot lies between the point ab

    // find perpendicular foot from circle center to line ab
    auto r = b-a;
    inet::Coord foot = a + r*( ((center - a)*r) / (r*r) );

    // check if perpendicular foot lies between points a and b
    // comparing m1/m2 not needed we know that foot lies on ab (but not where)
    double m1;
    if (std::abs(b.x - a.x) > 0){
        m1 = (foot.x - a.x) / (b.x - a.x);
    } else if (std::abs(b.y - a.y) > 0) {
        m1 = (foot.y - a.y) / (b.y - a.y);
    } else {
        throw cRuntimeError("intersection line is a point");
    }
    auto distSquared = std::pow(foot.x - center.x, 2) + std::pow(foot.y - center.y, 2);
    return  distSquared <= radiusSquared &&  m1 >= 0.0 && m1 <= 1.0;
}



bool GridCellIDKeyProvider::cellInRadius(const GridCellID& cell, const double radius, const traci::TraCIPosition& circleCenter) const {
    /*
     * note: Will not find all possible configurations but all necessary for selecting all cells within the radius
     * Assumptions:
     *  * cells are aligned wit circleCenter, meaning that the circleCenter will always coincide  with one
     *    cell.
     *  * situations where the circle is smaller than 1/4 of a square cell an the circle does not touch the center or
     *    any edge of the square will can not happen.
     */

    traci::TraCIPosition cellCenter = gridInfo.getCellCenter(cell);

    if ( (std::pow(cellCenter.x - circleCenter.x, 2) + std::pow(cellCenter.y - circleCenter.y, 2)) <= std::pow(radius, 2) )
        return true;

    inet::Coord cell_s =  gridInfo.getCellSize();

    inet::Coord a = inet::Coord(cellCenter.x + cell_s.x/2, cellCenter.y + cell_s.y/2);
    inet::Coord b = inet::Coord(cellCenter.x - cell_s.x/2, cellCenter.y + cell_s.y/2);
    inet::Coord c = inet::Coord(cellCenter.x - cell_s.x/2, cellCenter.y - cell_s.y/2);
    inet::Coord d = inet::Coord(cellCenter.x + cell_s.x/2, cellCenter.y - cell_s.y/2);

    inet::Coord cc{circleCenter.x, circleCenter.y, 0.0};
    double radiusSquared = std::pow(radius, 2);

    return  ((std::pow(a.x - cc.x, 2) + std::pow(a.y - cc.y, 2)) <= radiusSquared) ||
            ((std::pow(b.x - cc.x, 2) + std::pow(b.y - cc.y, 2)) <= radiusSquared) ||
            ((std::pow(c.x - cc.x, 2) + std::pow(c.y - cc.y, 2)) <= radiusSquared) ||
            ((std::pow(d.x - cc.x, 2) + std::pow(d.y - cc.y, 2)) <= radiusSquared) ||
            intersectLineCircle(a, b, radiusSquared, cc) ||
            intersectLineCircle(b, c, radiusSquared, cc) ||
            intersectLineCircle(c, d, radiusSquared, cc) ||
            intersectLineCircle(d, a, radiusSquared, cc);

}

std::vector<GridCellID> GridCellIDKeyProvider::getCellsInRadius(const inet::Coord& pos, double distance) const {
//    return getCellsInRadius(converter->position_cast_traci(pos), distance);
//}

//std::vector<GridCellID> GridCellIDKeyProvider::getCellsInRadius(const traci::TraCIPosition& pos, double distance) const {

    GridCellID cellId = getCellKey(pos);

    traci::TraCIPosition origCenter =  gridInfo.getCellCenter(cellId); // TracI coordinates needed here.

    // define enclosing square around the center of current cell with a radius of length 'distance'
    // If enclosing square is near lower bounds clip resulting square.
    double dist_epsilon = 0.1; // increase search range definition by 10 cm in each direction to include cells that
                               // cells in the lowerLeft corner if distance is equal to half of cell size.
                               // Each cell in the search range will be checked individually if they overlap with
                               // with the search radius. Additionally, ensure lower point to be Cell(0, 0).
    traci::TraCIPosition lowerLeft { std::max(0.0, origCenter.x - distance - dist_epsilon), std::max(0.0, origCenter.y - distance - dist_epsilon), 0.0};
    traci::TraCIPosition upperRight { origCenter.x + distance + dist_epsilon, origCenter.y + distance + dist_epsilon};

    GridCellID lowerCellId = getCellKey(lowerLeft);
    GridCellID upperCellId = getCellKey(upperRight);
    // Ensure upper search range is limited by map size
    upperCellId = GridCellID{
        std::min((int)(gridInfo.getCellCount().x-1), upperCellId.x()),
        std::min((int)(gridInfo.getCellCount().y-1), upperCellId.y())
    };


    // include all cells inside radius around center of originCell (not position in cell)
    // (i.e. if any of the cell corners or mid points is within the radius)
    std::vector<GridCellID> ret;
    for (int x = lowerCellId.x(); x <=upperCellId.x(); x++){
        for (int y = lowerCellId.y(); y  <= upperCellId.y(); y++){
            auto cell = GridCellID(x, y);
            if (this->cellInRadius(cell, distance, origCenter)){
                ret.push_back(cell);
            }
        }
    }
    return ret;
}


}  // namespace crownet
