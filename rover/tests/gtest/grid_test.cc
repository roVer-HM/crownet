/*
 * grid_test.cc
 *
 *  Created on: Sep 11, 2020
 *      Author: sts
 */

#include <gtest/gtest.h>
#include <string>
#include "rover/common/positionMap/DensityMeasure.h"
#include "rover/common/positionMap/Entry.h"

double TIME1 = 1.34;
double TIME2 = 4.42;

class IEntryTest : public ::testing::Test {
 protected:
  void SetUp() override {
    e1 = IEntry<int, int>();
    e2 = IEntry<double, double>(3, TIME1, TIME2);
  }

  IEntry<int, int> e1;
  IEntry<double, double> e2;
};

TEST_F(IEntryTest, Ctor1) {
  ASSERT_EQ(0, e1.getCount());
  ASSERT_EQ(0, e1.getMeasureTime());
  ASSERT_EQ(0, e1.getReceivedTime());
}

TEST_F(IEntryTest, Ctor2) {
  ASSERT_EQ(3, e2.getCount());
  ASSERT_EQ(TIME1, e2.getMeasureTime());
  ASSERT_EQ(TIME2, e2.getReceivedTime());
}

TEST_F(IEntryTest, isValid) {
  ASSERT_FALSE(e1.valid());
  ASSERT_TRUE(e2.valid());
}

TEST_F(IEntryTest, rest) {
  e1.reset();
  ASSERT_EQ(0, e1.getCount());
  ASSERT_EQ(0, e1.getMeasureTime());
  ASSERT_EQ(0, e1.getReceivedTime());
  ASSERT_FALSE(e1.valid());

  e2.reset();
  ASSERT_EQ(0, e2.getCount());
  ASSERT_EQ(TIME1, e2.getMeasureTime());
  ASSERT_EQ(TIME2, e2.getReceivedTime());
  ASSERT_FALSE(e2.valid());
}

TEST_F(IEntryTest, incrementCount) {
  e1.incrementCount(2);
  ASSERT_EQ(1, e1.getCount());
  ASSERT_EQ(2, e1.getMeasureTime());
  ASSERT_EQ(2, e1.getReceivedTime());
  ASSERT_TRUE(e1.valid());

  e2.incrementCount(2);
  ASSERT_EQ(4, e2.getCount());
  ASSERT_EQ(2, e2.getMeasureTime());
  ASSERT_EQ(2, e2.getReceivedTime());
  ASSERT_TRUE(e2.valid());
}

TEST_F(IEntryTest, compareMeasureTime) {
  IEntry<int, int> other1{1, 1, 1};
  IEntry<int, int> other2{1, 0, 1};
  ASSERT_LT(e1.compareMeasureTime(other1), 0);
  ASSERT_EQ(e1.compareMeasureTime(other2), 0);

  IEntry<double, double> other3{1, TIME1 - 0.4, 1.0};
  IEntry<double, double> other4{1, TIME1, 1};
  ASSERT_GT(e2.compareMeasureTime(other3), 0);
  ASSERT_EQ(e2.compareMeasureTime(other4), 0);
}

TEST_F(IEntryTest, toCsv) {
  const char *expectVal = "3,1.34,4.42";
  ASSERT_STREQ(expectVal, e2.csv(",").c_str());
}

TEST(LocalDensityMeasure, rest) {
  LocalDensityMeasure<int> m{};
  ASSERT_FALSE(m.valid());
  m.incrementCount(4.9);
  m.nodeIds.insert(32);
  ASSERT_TRUE(m.valid());
  m.reset();
  ASSERT_EQ(0, m.nodeIds.size());
  ASSERT_FALSE(m.valid());
}