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
#include <iterator> // For std::forward_iterator_tag
#include <cstddef>  // For std::ptrdiff_t

namespace crownet {

struct AoiIterator;

class RegularGridInfo {
public:
    RegularGridInfo(): gridSize(), cellSize(), cellCount() {};
    RegularGridInfo(const inet::Coord& gridSize, const inet::Coord& cellSize, const traci::Boundary aoi);
    RegularGridInfo(const inet::Coord& gridSize, const inet::Coord& cellSize);
    RegularGridInfo(const traci::Boundary simBound, const inet::Coord& gridSize, const inet::Coord& cellSize, const traci::Boundary aoi);
    RegularGridInfo(const traci::Boundary simBound, const inet::Coord& gridSize, const inet::Coord& cellSize);
    virtual ~RegularGridInfo()=default;

    const inet::Coord& getGridSize() const { return gridSize; }
    const inet::Coord& getBound() const { return gridSize; }
    const inet::Coord& getCellSize() const { return cellSize; }
    const inet::Coord& getCellCount() const { return cellCount;}
    const traci::Boundary& getAreaOfIntrest() const { return areaOfIntrest; }
    void setGridSize(const inet::Coord gSize) { gridSize = gSize; }
    void setCellSize(const inet::Coord cSize) { cellSize = cSize; }
    void setCellCount(const inet::Coord cCount) { cellCount = cCount;}
    void setAreaOfIntrest(const traci::Boundary b) {areaOfIntrest = b;}
    void setSimBound(const traci::Boundary sb) {simBound = sb; }
    const AoiIterator aoiIter() const;

    const traci::TraCIPosition getCellCenter(const GridCellID& cell) const {return getCellCenter(cell.x(), cell.y());}
    const traci::TraCIPosition getCellCenter(const int x, const int y) const;

    const GridCellID getCellKey(const traci::TraCIPosition& pos) const;
    const GridCellID getCellKey(const int cellKey1D) const;

    const int getCellKey1D(const GridCellID& cellKey) const;
    const int getCellKey1D(const traci::TraCIPosition& pos) const;
    const int getCellKey1D(const int x, const int y) const;

    const bool posInCenteredCell(const inet::Coord& cellCenter, const inet::Coord& pos ) const;

    double cellCenterDist(const GridCellID& cell1, const GridCellID&  cell2)const;
    double maxCellDist(const GridCellID& cell1, const GridCellID&  cell2) const;


private:
    inet::Coord gridSize;   // todo: assume gridSize == simBound!
    traci::Boundary simBound;
    traci::Boundary areaOfIntrest;
    inet::Coord cellSize;   //
    inet::Coord cellCount;
};

struct AoiIterator {
    using iterator_category = std::input_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = GridCellID;
    using pointer           = GridCellID*;  // or also value_type*
    using reference         = const GridCellID&;  // or also value_type&



    AoiIterator(const RegularGridInfo& grid){
        m_ptr = grid.getCellKey(grid.getAreaOfIntrest().lowerLeftPosition());
        aoi.x_min = m_ptr.x();
        aoi.y_min = m_ptr.y();
        aoi.x_max = grid.getCellKey(grid.getAreaOfIntrest().upperRightPosition()).x()-1;
        aoi.y_max = grid.getCellKey(grid.getAreaOfIntrest().upperRightPosition()).y()-1;
    }


    reference operator*() const { return m_ptr; }
    pointer operator->() { return &m_ptr; }

    // Prefix increment
    AoiIterator& operator++() {
        (*this)++;
        return *this;
    }

    // Postfix increment
    AoiIterator operator++(int) {
        AoiIterator tmp = *this;
        if (m_ptr.x() < aoi.x_max){
            m_ptr=GridCellID(m_ptr.x()+1, m_ptr.y());
        } else if (m_ptr.y() < aoi.y_max){
            m_ptr=GridCellID(aoi.x_min, m_ptr.y()+1);
        } else  {
            m_ptr=GridCellID(-1, -1);
        }
        auto c = GridCellID();
        return tmp;
    }
    AoiIterator begin() const { return AoiIterator(aoi); }
    AoiIterator end() const  { return AoiIterator(); }

    friend bool operator== (const AoiIterator& a, const AoiIterator& b) { return a.m_ptr == b.m_ptr; };
    friend bool operator!= (const AoiIterator& a, const AoiIterator& b) { return a.m_ptr != b.m_ptr; };
private:
    struct AOI {
        int x_min = -1;
        int x_max = -1;
        int y_min = -1;
        int y_max = -1;
    };
    AoiIterator(): m_ptr(GridCellID(-1, -1)), aoi() {}
    AoiIterator(const AOI& aoi): m_ptr(GridCellID(aoi.x_min, aoi.y_min)), aoi(aoi) {}


private:
    GridCellID m_ptr;
    AOI aoi;

};

};

