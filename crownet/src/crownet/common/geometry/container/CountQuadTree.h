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

#include <algorithm>
#include "inet/common/geometry/common/Coord.h"
#include "crownet/common/geometry/container/FreeList.h"

using namespace inet;

namespace crownet {


struct ElementNode {
    const cObject *elementData;
    Coord pos;
    int next;
};

// Quadrant of the QuadTree
struct QuadTreeNode {
    // If branch node (count = -1): index of first child node in the QuadTree (treeNodes vector)
    // If leaf node (count != -1): index of fist element node (data vector)
    Coord boundaryMin;
    Coord boundaryMax;
    int firstChild = -1;
    int count = 0;
    int level = 0;

    bool contains(const Coord& pos) const;
    bool isLeaf() const {return count != -1;}
    bool isBranch() const {return count == -1;}
    bool isEmptyLeaf() const {return firstChild == -1;}
    void setBoundary(Coord *minBoundaries, Coord *maxBoundaries) const;
};



class CountQuadTree  {
public:

    std::vector<QuadTreeNode> treeNodes;

    int quadrantCapacity;
    int max_depth;
    int free_node;


    int insert(const cObject* data, const Coord& pos);
    std::vector<ElementNode> getList(int qNodeIdx);
    int dataSize() const { return data.size(); }

//    void move(const cObject* data, const Coord& newPos);
//    void move(const int& dataId, const Coord& newPos);
//    void remove(const cObject* data);
//    void remove(const int& dataId);

private:
    FreeList<ElementNode>  data;


private:
    void splitNode(QuadTreeNode& node);
    int getQuadrant(QuadTreeNode& node, const Coord& pos);

public:
    CountQuadTree(Coord boundMin, Coord boundMax, int quadrantCapacity=1, int max_depth=10);
    CountQuadTree(Coord boundMin, double width, double height, int quadrantCapacity=1, int max_depth=10);
    virtual ~CountQuadTree() = default;
};

}
