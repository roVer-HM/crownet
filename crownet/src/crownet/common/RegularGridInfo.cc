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
}

RegularGridInfo::RegularGridInfo(const traci::Boundary simBound, const inet::Coord& gridSize, const inet::Coord& cellSize)
  : gridSize(gridSize), cellSize(cellSize), simBound(simBound){

    cellCount.x = std::floor(gridSize.x/cellSize.x);
    cellCount.y = std::floor(gridSize.y/cellSize.y);
    cellCount.z = 0.0;
}

const inet::Coord RegularGridInfo::getCellCenter(const int x, const int y) const{
    return inet::Coord(x * cellSize.x + cellSize.x/2, y * cellSize.y + cellSize.y/2, 0.0);
}


const int RegularGridInfo::getCellId(const int x, const int y)const{
    // row major order
    if (x < 0 || x >= cellCount.x){
        throw omnetpp::cRuntimeError("Cell Id out of bound");
    }
    return y*cellCount.x + x;
}

const int RegularGridInfo::getCellId(inet::Coord position) const{
    return getCellId((int)position.x, (int)position.y);
}

double RegularGridInfo::cellCenterDist(const GridCellID& cell1, const GridCellID&  cell2) const {
    inet::Coord c1 {cell1.x()*cellSize.x + cellSize.x/2, cell1.y()*cellSize.y + cellSize.y/2};
    inet::Coord c2 {cell2.x()*cellSize.x + cellSize.x/2, cell2.y()*cellSize.y + cellSize.y/2};
    return c1.distance(c2);
}

double RegularGridInfo::maxCellDist(const GridCellID& cell1, const GridCellID&  cell2) const {
    return std::max(std::abs(cell1.x() - cell2.x()) * cellSize.x, std::abs(cell1.y() - cell2.y()) * cellSize.y);
}

int RegularGridInfo::maxIdCellDist(const GridCellID& cell1, const GridCellID&  cell2) const {
    return std::max(std::abs(cell1.x() - cell2.x()), std::abs(cell1.y() - cell2.y()));
}


}
