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

#include "RegularGridInfo.h"
#include <omnetpp/cexception.h>


namespace crownet {

RegularGridInfo::RegularGridInfo(const inet::Coord& gridSize, const inet::Coord& cellSize)
  : gridSize(gridSize), cellSize(cellSize){

    cellCount.x = std::floor(gridSize.x/cellSize.x);
    cellCount.y = std::floor(gridSize.y/cellSize.y);
    cellCount.z = 0.0;

    std::vector<traci::TraCIPosition> vec;
    traci::TraCIPosition upperRight{0.0, 0.0, 0.0};
    upperRight.x = gridSize.x;
    upperRight.y = gridSize.y;
    vec.push_back(traci::TraCIPosition{0.0, 0.0, 0.0});
    vec.push_back(upperRight);
    simBound = traci::Boundary(vec);
    areaOfIntrest = traci::Boundary(vec);
}
RegularGridInfo::RegularGridInfo(const inet::Coord& gridSize, const inet::Coord& cellSize, const traci::Boundary aoi)
  : gridSize(gridSize), areaOfIntrest(aoi), cellSize(cellSize){

    cellCount.x = std::floor(gridSize.x/cellSize.x);
    cellCount.y = std::floor(gridSize.y/cellSize.y);
    cellCount.z = 0.0;

    std::vector<traci::TraCIPosition> vec;
    traci::TraCIPosition upperRight{0.0, 0.0, 0.0};
    upperRight.x = gridSize.x;
    upperRight.y = gridSize.y;
    vec.push_back(traci::TraCIPosition{0.0, 0.0, 0.0});
    vec.push_back(upperRight);
    simBound = traci::Boundary(vec);
}

RegularGridInfo::RegularGridInfo(const traci::Boundary simBound, const inet::Coord& gridSize, const inet::Coord& cellSize)
  : RegularGridInfo(simBound, gridSize, cellSize, simBound){
}

RegularGridInfo::RegularGridInfo(const traci::Boundary simBound, const inet::Coord& gridSize, const inet::Coord& cellSize, const traci::Boundary aoi)
  : gridSize(gridSize), simBound(simBound), areaOfIntrest(aoi), cellSize(cellSize){

    cellCount.x = std::floor(gridSize.x/cellSize.x);
    cellCount.y = std::floor(gridSize.y/cellSize.y);
    cellCount.z = 0.0;
}
const AoiIterator RegularGridInfo::aoiIter() const{
    return AoiIterator(*this);
}

const traci::TraCIPosition RegularGridInfo::getCellCenter(const int x, const int y) const{
    return traci::TraCIPosition(x * cellSize.x + cellSize.x/2, y * cellSize.y + cellSize.y/2, 0.0);
}

const GridCellID RegularGridInfo::getCellKey(const traci::TraCIPosition& pos) const{
    int x_id = (int)floor(pos.x / cellSize.x);
    int y_id = (int)floor(pos.y / cellSize.y);
    return GridCellID(x_id, y_id);
}

const GridCellID RegularGridInfo::getCellKey(const int cellKey1D) const { //todo Test
    int _id = (int)abs(cellKey1D);
    int x_id = (int)floor(_id % (int)cellCount.x);
    int y_id = (int)floor(_id / (int)cellCount.x);
    return GridCellID(x_id, y_id);
}

const int RegularGridInfo::getCellKey1D(const traci::TraCIPosition& pos) const{
    auto id = getCellKey(pos);
    return getCellKey1D(id.x(), id.y());
}
const int RegularGridInfo::getCellKey1D(const int x, const int y) const{
    // row major order
    if (x < 0 || x >= cellCount.x){
        throw omnetpp::cRuntimeError("Cell Id out of bound");
    }
    return y*cellCount.x + x;
}
const int RegularGridInfo::getCellKey1D(const GridCellID& cellKey) const{
    return getCellKey1D(cellKey.x(), cellKey.y());
}

const bool RegularGridInfo::posInCenteredCell(const inet::Coord& cellCenter, const inet::Coord& pos ) const {
    return pos.x >= cellCenter.x-cellSize.x/2 &&
            pos.x < cellCenter.x+cellSize.x/2 &&
            pos.y >= cellCenter.y-cellSize.y/2 &&
            pos.y < cellCenter.y+cellSize.y/2;
}

double RegularGridInfo::cellCenterDist(const GridCellID& cell1, const GridCellID&  cell2) const {
    inet::Coord c1 {cell1.x()*cellSize.x + cellSize.x/2, cell1.y()*cellSize.y + cellSize.y/2};
    inet::Coord c2 {cell2.x()*cellSize.x + cellSize.x/2, cell2.y()*cellSize.y + cellSize.y/2};
    return c1.distance(c2);
}

double RegularGridInfo::maxCellDist(const GridCellID& cell1, const GridCellID&  cell2) const {
    return std::max(std::abs(cell1.x() - cell2.x()) * cellSize.x, std::abs(cell1.y() - cell2.y()) * cellSize.y);
}


}
