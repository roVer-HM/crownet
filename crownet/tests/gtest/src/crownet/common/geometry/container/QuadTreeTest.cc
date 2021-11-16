/*
 * QuadTreeTest.cc
 *
 *  Created on: Oct 14, 2021
 *      Author: vm-sts
 */




#include <gtest/gtest.h>
#include <omnetpp.h>
#include <memory>
#include <string>

#include "crownet/common/geometry/container/FreeList.h"
#include "crownet/common/geometry/container/CountQuadTree.h"

using namespace crownet;
using namespace inet;

class cInt : public cObject {

public:
    cInt(int i) : cObject(), val(i){}
    cInt() : cObject(), val(-1){}

    friend bool operator==(const cInt& a, const cInt& b){
        return a.val == b.val;
    }

    int val;
};

class QuadTreeTestF : public ::testing::Test {
protected:
    void SetUp() override {
        int capacity = 1;
        qtree = new CountQuadTree(Coord{0.0, 0.0}, 64.0, 64.0, capacity);
    }

    void TearDown() override {
        for(auto o : data){
            delete o;
        }
        delete qtree;
    }

    cObject* getData(){
        auto o = new cInt(data.size());
        data.push_back(o);
        return o;
    }

    void resetCapacit(int capacity){
        delete qtree;
        qtree = new CountQuadTree(Coord{0.0, 0.0}, 64.0, 64.0, capacity);
    }

    CountQuadTree* qtree;
    std::vector<cObject*> data;

    // test functions

    void testLeaf(CountQuadTree* t, int id){
        EXPECT_TRUE(t->treeNodes[id].isLeaf()) << "  ==> For id: " << id;
    }

    void testBranch(CountQuadTree* t, int id){
        EXPECT_FALSE(t->treeNodes[id].isLeaf()) << "  ==> For id: " << id;
    }

    void testCount(CountQuadTree* t, int id, int count){
        EXPECT_EQ(t->treeNodes[id].count, count) << "  ==> For id: " << id;
    }

    void testObjectsInQuadNode(CountQuadTree* t, int id, std::vector<int> obj){
        std::vector<ElementNode> elements = t->getList(id);
        for(int i = 0; i< obj.size(); i++){
            cInt* a = (cInt*)elements[i].elementData;
            cInt* b = (cInt*)data[obj[i]];
            EXPECT_TRUE(a == b) << "  Failed on object index: " << i << "for node id: " << id;
        }
    }
};

TEST_F(QuadTreeTestF, insertion1){

    qtree->insert(getData(), {4.5, 4.5});
    EXPECT_EQ(qtree->dataSize(), 1);
    EXPECT_EQ(qtree->treeNodes.size(), 1);
    EXPECT_EQ(qtree->treeNodes[0].count, 1);

}

TEST_F(QuadTreeTestF, insertion2){

    qtree->insert(getData(), {4.5, 4.5});
    qtree->insert(getData(), {35.0, 35.0});
    EXPECT_EQ(qtree->dataSize(), 2);
    EXPECT_EQ(qtree->treeNodes.size(), 5); // root + 4 nodes (due to capacity = 1)

    // place one item in each quadrant
    qtree->insert(getData(), {35.0, 4.5});
    qtree->insert(getData(), {4.5, 35.0});
    EXPECT_EQ(qtree->dataSize(), 4);
    EXPECT_EQ(qtree->treeNodes.size(), 5); // root + 4 nodes (due to capacity = 1)
}

TEST_F(QuadTreeTestF, insertion3){

    qtree->insert(getData(), {4.5, 4.5});
    qtree->insert(getData(), {9.0, 9.0});
    EXPECT_EQ(qtree->dataSize(), 2);
    EXPECT_EQ(qtree->treeNodes.size(), 1+3*4);

}

TEST_F(QuadTreeTestF, insertion4){

    qtree->insert(getData(), {4.5, 4.5});
    qtree->insert(getData(), {7.0, 7.0});
    EXPECT_EQ(qtree->dataSize(), 2);
    EXPECT_EQ(qtree->treeNodes.size(), 1+5*4);

}

TEST_F(QuadTreeTestF, insertion1_cap4){
    resetCapacit(4);
    // 4 items in Q1 (left top->left bottom)
    qtree->insert(getData(), {4.5, 35.5});
    qtree->insert(getData(), {10.0, 40.5});
    qtree->insert(getData(), {14.0, 45.5});
    qtree->insert(getData(), {28.0, 50.5});
    EXPECT_EQ(qtree->dataSize(), 4);
    EXPECT_EQ(qtree->treeNodes.size(), 1);

    // one more in Q3
    qtree->insert(getData(), {40.0, 20.5});
    EXPECT_EQ(qtree->dataSize(), 5);
    EXPECT_EQ(qtree->treeNodes.size(), 1+4);

    testBranch(qtree, 0);   // root is now a branch
    testLeaf(qtree, 1);     // Quad1 contains 4 items
    testCount(qtree, 1, 4);
    testLeaf(qtree, 2);     // Quad2 nothing
    testCount(qtree, 2, 0);
    testLeaf(qtree, 3);     // Quad3 the 5th item that causes the split
    testCount(qtree, 3, 1);
    testLeaf(qtree, 4);     // Quad4 nothing
    testCount(qtree, 4, 0);

    testObjectsInQuadNode(qtree, 1, {0,1,2,3});
    testObjectsInQuadNode(qtree, 3, {4});
}
