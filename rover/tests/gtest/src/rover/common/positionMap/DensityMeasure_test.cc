/*
 * grid_test.cc
 *
 *  Created on: Sep 11, 2020
 *      Author: sts
 */

#include "rover/common/positionMap/DensityMeasure.h"
#include <gtest/gtest.h>
#include <string>

// rest must also rest nodeIds set
TEST(LocalDensityMeasure, rest) {
  LocalDensityMeasure<int> m{};
  EXPECT_FALSE(m.valid());
  m.incrementCount(4.9);
  m.nodeIds.insert(32);
  EXPECT_TRUE(m.valid());
  m.reset();
  EXPECT_EQ(0, m.nodeIds.size());
  EXPECT_FALSE(m.valid());
}

TEST(LocalDensityMeasure, csv) {
  LocalDensityMeasure<int> m{1, 3.5, 3.7};
  std::string csv = "1,3.5,3.7";
  EXPECT_STREQ(csv.c_str(), m.csv(",").c_str());
  csv = "1;3.5;3.7";
  EXPECT_STREQ(csv.c_str(), m.csv(";").c_str());
}

TEST(LocalDensityMeasure, empty) {
  LocalDensityMeasure<int> m1{-1, 3.5, 3.7};
  EXPECT_TRUE(m1.empty());
  LocalDensityMeasure<int> m2{0, 3.5, 3.7};
  EXPECT_FALSE(m2.empty());
  LocalDensityMeasure<int> m3{1, 3.5, 3.7};
  EXPECT_FALSE(m3.empty());
}
