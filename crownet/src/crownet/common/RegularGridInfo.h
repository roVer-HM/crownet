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

#pragma once

#include "inet/common/geometry/Geometry_m.h"
#include "crownet/dcd/identifier/Identifiers.h"
#include <artery/traci/Cast.h>

namespace crownet {

class RegularGridInfo {
public:
    RegularGridInfo(): gridSize(), cellSize(), cellCount() {};
    RegularGridInfo(const inet::Coord& gridSize, const inet::Coord& cellSize);
    RegularGridInfo(const traci::Boundary simBound, const inet::Coord& gridSize, const inet::Coord& cellSize);
    virtual ~RegularGridInfo()=default;

    const inet::Coord& getGridSize() const { return gridSize; }
    const inet::Coord& getBound() const { return gridSize; }
    const inet::Coord& getCellSize() const { return cellSize; }
    const inet::Coord& getCellCount() const { return cellCount;}
    void setGridSize(const inet::Coord gSize) { gridSize = gSize; }
    void setCellSize(const inet::Coord cSize) { cellSize = cSize; }
    void setCellCount(const inet::Coord cCount) { cellCount = cCount;}

    const inet::Coord getCellCenter(const int x, const int y) const;
    const inet::Coord getOppCellCenter(inet::Coord position) const;

    const int getCellId(const int x, const int y)const;
    const int getCellId(inet::Coord position) const;

    double cellCenterDist(const GridCellID& cell1, const GridCellID&  cell2)const;
    double maxCellDist(const GridCellID& cell1, const GridCellID&  cell2) const;
    int maxIdCellDist(const GridCellID& cell1, const GridCellID&  cell2) const;


private:
    inet::Coord gridSize;   // todo: assume gridSize == simBound!
    traci::Boundary simBound;
    inet::Coord cellSize;   //
    inet::Coord cellCount;


};


};

