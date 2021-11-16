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

#include "CountQuadTree.h"

using namespace inet;

namespace crownet {

CountQuadTree::CountQuadTree(Coord boundMin, Coord boundMax, int quadrantCapacity, int max_depth)
           :quadrantCapacity(quadrantCapacity),
            max_depth(max_depth),
            free_node(-1){
    QuadTreeNode root {boundMin, boundMax, -1, 0, 0}; // firstChild=-1, count=0 , level=0
    treeNodes.push_back(root);
}

CountQuadTree::CountQuadTree(Coord boundMin, double width, double height, int quadrantCapacity, int max_depth)
        :CountQuadTree(boundMin, {boundMin.x + width, boundMin.y + height}, quadrantCapacity ,max_depth) {}


int CountQuadTree::insert(const cObject* elementData, const Coord& pos){

    std::vector<int> treeNodesToProcess;
    treeNodesToProcess.push_back(0);
    while(treeNodesToProcess.size() > 0){
        int nodeIdx = treeNodesToProcess.back();
        treeNodesToProcess.pop_back();
        QuadTreeNode& node = treeNodes[nodeIdx];
        if (node.contains(pos)){
            if (node.isLeaf()){
                if (node.count < quadrantCapacity){
                    // add point to quadrant
                    ElementNode eNode;
                    eNode.elementData = elementData;
                    eNode.pos = pos;
                    eNode.next = node.firstChild;
                    node.firstChild = data.insert(eNode);
                    ++node.count;
                    treeNodes[nodeIdx] = node;
                    return node.firstChild;
                } else {
                    // split and move points
                    splitNode(node);
                    node.count = -1; // node is now a branch node.
                    treeNodes[nodeIdx] = node;
                    // check children to insert
                    treeNodesToProcess.push_back(node.firstChild);
                    treeNodesToProcess.push_back(node.firstChild+1);
                    treeNodesToProcess.push_back(node.firstChild+2);
                    treeNodesToProcess.push_back(node.firstChild+3);
                }
            } else {
                // check children insert
                treeNodesToProcess.push_back(node.firstChild);
                treeNodesToProcess.push_back(node.firstChild+1);
                treeNodesToProcess.push_back(node.firstChild+2);
                treeNodesToProcess.push_back(node.firstChild+3);
            }
        }
    }
    throw cRuntimeError("Insertion failed for object: %s with position: (%f, %f, %f)", elementData->getFullName(), pos.x, pos.y, pos.z);
}

std::vector<ElementNode> CountQuadTree::getList(int qNodeIdx){
    std::vector<ElementNode> elList;
    QuadTreeNode& node = treeNodes[qNodeIdx];
    int next = node.firstChild;
    while(next != -1){
        auto e = data[next];
        next = e.next;
        elList.push_back(e);
    }
    return elList;
}


void CountQuadTree::splitNode(QuadTreeNode& node){
    // create 4 new bounding boxes
    Coord minBoundaries[4], maxBoundaries[4];
    node.setBoundary(minBoundaries, maxBoundaries);
    // create children as leafs
    for(int i=0; i < 4; i++){
        QuadTreeNode childNode;
        childNode.boundaryMin = minBoundaries[i];
        childNode.boundaryMax = maxBoundaries[i];
        childNode.firstChild = -1; // new leaf node with zero data elements
        childNode.count  = 0; // new leaf node with zero data elements
        childNode.level = node.level + 1;
        if (free_node != -1){
            treeNodes[free_node + i] = childNode;
        } else {
            treeNodes.push_back(childNode);
        }
    }
    // set first_child of node (now branch node) to the first child.
    int elementIndex = node.firstChild; // save ElementNode index before overriding with QuadNode index.
    if (free_node != -1){
        node.firstChild = free_node;
        free_node = -1;
    } else {
        node.firstChild = treeNodes.size()-4;
    }
    // move data points to new children

    while(elementIndex != -1){
        ElementNode& eNode = data[elementIndex];
        // find child node for quadrant
        int quadrant = getQuadrant(node, eNode.pos);
        QuadTreeNode& newQuad = treeNodes[quadrant];

        int tmp = newQuad.firstChild;  // save first element of newQuadrant
        newQuad.firstChild = elementIndex; // add current element as new first element
        elementIndex = eNode.next; // update next child to be moved
        eNode.next = tmp; // connect new first element to the rest of the items in the new Quadrant
        ++newQuad.count; // increment node count managed by this leafe
        treeNodes[quadrant] = newQuad;
    }
}

int CountQuadTree::getQuadrant(QuadTreeNode& node, const Coord& pos){
    for (int i=0; i < 4; i++){
        if (treeNodes[node.firstChild + i].contains(pos)){
            return node.firstChild + i;
        }
    }
    throw cRuntimeError("Coord (%f, %f, %f) not in bound (%f, %f, %f, %f)", pos.x, pos.y, pos.z,
            node.boundaryMin.x, node.boundaryMin.y, node.boundaryMax.x, node.boundaryMax.y);

}



bool QuadTreeNode::contains(const Coord& pos) const {
    return pos.x <= boundaryMax.x && pos.x >= boundaryMin.x &&
           pos.y <= boundaryMax.y && pos.y >= boundaryMin.y;
}

void QuadTreeNode::setBoundary(Coord *minBoundaries, Coord *maxBoundaries) const{
    // We just divide a rectangle into four smaller congruent rectangle
    // see inet::QuadTree
     maxBoundaries[0].x = (boundaryMax.x - boundaryMin.x) / 2 + boundaryMin.x;
     minBoundaries[0].y = (boundaryMax.y - boundaryMin.y) / 2 + boundaryMin.y;
     minBoundaries[0].x = boundaryMin.x;
     maxBoundaries[0].y = boundaryMax.y;

     minBoundaries[1].x = (boundaryMax.x - boundaryMin.x) / 2 + boundaryMin.x;
     minBoundaries[1].y = (boundaryMax.y - boundaryMin.y) / 2 + boundaryMin.y;
     maxBoundaries[1].x = boundaryMax.x;
     maxBoundaries[1].y = boundaryMax.y;

     minBoundaries[2].x = (boundaryMax.x - boundaryMin.x) / 2 + boundaryMin.x;
     maxBoundaries[2].y = (boundaryMax.y - boundaryMin.y) / 2 + boundaryMin.y;
     maxBoundaries[2].x = boundaryMax.x;
     minBoundaries[2].y = boundaryMin.y;

     maxBoundaries[3].y = (boundaryMax.y - boundaryMin.y) / 2 + boundaryMin.y;
     maxBoundaries[3].x = (boundaryMax.x - boundaryMin.x) / 2 + boundaryMin.x;
     minBoundaries[3].x = boundaryMin.x;
     minBoundaries[3].y = boundaryMin.y;

}

}

