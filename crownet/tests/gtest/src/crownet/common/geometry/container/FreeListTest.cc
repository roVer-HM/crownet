/*
 * FreeListTest.h
 *
 *  Created on: Oct 14, 2021
 *      Author: vm-sts
 */

#include <gtest/gtest.h>
#include <omnetpp.h>
#include <memory>
#include <string>

#include "crownet/common/geometry/container/FreeList.h"

class FreeListTestF : public ::testing::Test {
protected:
    FreeList<double> dList;
    FreeList<std::string> sList;
};

TEST_F(FreeListTestF, empty){
    EXPECT_EQ(dList.size(), 0);
    EXPECT_EQ(sList.size(), 0);
}

TEST_F(FreeListTestF, add){
    dList.insert(2.2);
    dList.insert(3.3);
    dList.insert(4.4);
    EXPECT_EQ(dList.size(), 3);
}

TEST_F(FreeListTestF, access){
    sList.insert("Hello");
    sList.insert("World");
    sList.insert("!");
    EXPECT_STREQ(sList[0].c_str(), "Hello");
    EXPECT_STREQ(sList[1].c_str(), "World");
    EXPECT_STREQ(sList[2].c_str(), "!");
}

TEST_F(FreeListTestF, erase){
    dList.insert(2.2);
    dList.insert(3.3);
    dList.insert(4.4);
    dList.erase(2);
    EXPECT_EQ(dList.size(), 2);
}

TEST_F(FreeListTestF, eraseInsert){
    dList.insert(2.2);
    dList.insert(3.3);
    dList.insert(4.4);
    dList.erase(1);
    dList.insert(5.5);
    EXPECT_EQ(dList.size(), 3);
    EXPECT_DOUBLE_EQ(dList[1], 5.5);
}

TEST_F(FreeListTestF, multiInsertErease){
    for (int i=0; i < 30; i++){
        dList.insert(i * 1.0);
    }
    EXPECT_EQ(dList.size(), 30);
    dList.erase(4);
    dList.erase(20);
    dList.erase(17);
    dList.erase(13);
    EXPECT_EQ(dList.size(), 26);

    dList.insert(-1.0);
    dList.insert(-2.0);
    dList.insert(-3.0);
    dList.insert(-4.0);
    EXPECT_EQ(dList.size(), 30);
    dList.insert(-5.0); // no free space left push_back
    EXPECT_EQ(dList.size(), 31);
    EXPECT_DOUBLE_EQ(dList[4], -4.0);
    EXPECT_DOUBLE_EQ(dList[20], -3.0);
    EXPECT_DOUBLE_EQ(dList[17], -2.0);
    EXPECT_DOUBLE_EQ(dList[13], -1.0);
    EXPECT_DOUBLE_EQ(dList[dList.capacity()-1], -5.0);

}

TEST_F(FreeListTestF, accessInvalid){
    sList.insert("Hello");
    sList.insert("World");
    sList.insert("!");
    sList.erase(0);
    try {
        std::string x = sList[0];
        FAIL();
    } catch (omnetpp::cRuntimeError e) {

    }
}

TEST_F(FreeListTestF, iterator1){
    for (int i=0; i < 30; i++){
        dList.insert(i * 1.0);
    }
    int j = 0;
    for(const auto& item : dList){
        EXPECT_DOUBLE_EQ(item, j*1.0);
    }
}

TEST_F(FreeListTestF, iterator2){
    for (int i=0; i < 10; i++){
        dList.insert(i * 1.0);
    }
    dList.erase(3);
    dList.erase(7);
    dList.erase(6);
    dList.erase(5);
    dList.erase(0);
    int i = 0;
    std::vector<double> val = {1.0, 2.0, 4.0, 8.0, 9.0};
    for(const auto& item : dList){
        EXPECT_DOUBLE_EQ(item,val[i++]);
    }
}
