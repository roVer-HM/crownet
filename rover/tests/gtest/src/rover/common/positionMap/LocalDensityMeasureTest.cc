/*
 * grid_test.cc
 *
 *  Created on: Sep 11, 2020
 *      Author: sts
 */

#include <gtest/gtest.h>
#include <omnetpp.h>
#include <string>
#include "rover/common/positionMap/Entry.h"

using LocalEntry = ILocalEntry<int, omnetpp::simtime_t>;

// rest must also rest nodeIds set
TEST(LocalDensityMeasureTest, rest) {
  LocalEntry m{};
  EXPECT_TRUE(m.valid());
  m.incrementCount(4.9);
  m.nodeIds.insert(32);
  EXPECT_TRUE(m.valid());
  m.reset(1.0);
  EXPECT_EQ(0, m.nodeIds.size());
  EXPECT_FALSE(m.valid());
}

TEST(LocalDensityMeasureTest, csv) {
  LocalEntry m{1, 3.5, 3.7};
  std::string csv = "1,3.5,3.7,0,";  // empty selected in
  EXPECT_STREQ(csv.c_str(), m.csv(",").c_str());
  csv = "1;3.5;3.7;0;";  // empty selected in
  EXPECT_STREQ(csv.c_str(), m.csv(";").c_str());
}

TEST(LocalDensityMeasureTest, empty) {
  LocalEntry m1{-1, 3.5, 3.7};
  EXPECT_TRUE(m1.empty());
  LocalEntry m2{0, 3.5, 3.7};
  EXPECT_FALSE(m2.empty());
  LocalEntry m3{1, 3.5, 3.7};
  EXPECT_FALSE(m3.empty());
}
