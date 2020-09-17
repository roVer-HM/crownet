/*
 * RegularGridFullIter_test.cc
 *
 *  Created on: Sep 17, 2020
 *      Author: sts
 */

#include <gtest/gtest.h>
#include <boost/range/algorithm.hpp>
#include <memory>
#include <string>
#include "rover/common/positionMap/DensityMeasure.h"
#include "rover/common/positionMap/RegularGridMap.h"

using namespace rover;

class RegularGridMapFullIterTest : public ::testing::Test {
 protected:
  void SetUp() override {
    g1 = std::make_shared<RegularGridMap<std::string>>("Id1", 5.0,
                                                       std::make_pair(4, 3));
    g1->incrementLocalOwnPos(coord2cell2_1, 3.2);
    g1->incrementLocal(coord1cell2_1, "NodeA", 3.2);
    g1->incrementLocal(coord1cell2_1, "NodeB", 3.2);
    g1->incrementLocal(coord2cell2_1, "NodeC", 3.2);
    // cell(1,1): local measure: (2, 3.1, 3.1)
    g1->incrementLocal(coord3cell1_1, "NodeD", 3.1);
    g1->incrementLocal(coord3cell1_1, "NodeC", 3.1);

    // data received from NodeE with.
    g1->update(std::make_pair(3, 2), "NodeE", std::move(this->m(9, 5.3, 5.27)));
    g1->update(std::make_pair(0, 0), "NodeE", std::move(this->m(7, 5.3, 5.07)));
  }

  std::shared_ptr<DensityMeasure<std::string>> m(int count, double time1,
                                                 double time2) {
    return std::make_shared<DensityMeasure<std::string>>(count, time1, time2);
  }

  inet::Coord coord1cell2_1{13.0, 6.0};  // (2,1) rowMajorOrder
  inet::Coord coord2cell2_1{13.5, 7.7};  // (2,1) rowMajorOrder
  inet::Coord coord3cell1_1{5.5, 7.8};   // (1,1) rowMajorOrder
  std::shared_ptr<RegularGridMap<std::string>> g1;
};

TEST_F(RegularGridMapFullIterTest, getKey_rowMajorOrder) {
  auto iter = g1->iter("ymf", true);  //
  EXPECT_EQ(std::make_pair(0, 0), iter.getKey());
  iter++;
  EXPECT_EQ(std::make_pair(1, 0), iter.getKey());
  iter++;  // (2, 0)
  iter++;  // (3, 0)
  iter++;  // (0, 1)
  EXPECT_EQ(std::make_pair(0, 1), iter.getKey());
  iter++;  // (1, 1)
  EXPECT_EQ(std::make_pair(1, 1), iter.getKey());
  iter++;
  EXPECT_EQ(std::make_pair(2, 1), iter.getKey());
}

TEST_F(RegularGridMapFullIterTest, getKey_columnMajorOrder) {
  auto iter = g1->iter("ymf", false);
  EXPECT_EQ(std::make_pair(0, 0), iter.getKey());
  iter++;  // (0, 1)
  EXPECT_EQ(std::make_pair(0, 1), iter.getKey());
  iter++;  // (0, 2)
  iter++;  // (1, 0)
  iter++;  // (1, 1)
  EXPECT_EQ(std::make_pair(1, 1), iter.getKey());
  iter++;  // (1, 2)
  EXPECT_EQ(std::make_pair(1, 2), iter.getKey());
  iter++;
  EXPECT_EQ(std::make_pair(2, 0), iter.getKey());
}

TEST_F(RegularGridMapFullIterTest, out_of_range) {
  auto iter = g1->iter("ymf", true);
  bool catched = false;
  for (int i = 0; i < 30; i++) {
    try {
      iter++;
    } catch (std::out_of_range& e) {
      catched = true;
      EXPECT_EQ(iter.getMaxIndex(), i);
      break;
    }
  }
  EXPECT_TRUE(catched);
}

/**
 * (0,0) in lower left ( . := no value (-1))
 * -----------------
 * | . | . | . | 9 |
 * | . | 2 | . | . |
 * | 7 | . | . | . |
 * -----------------
 *
 * Cells in rowMajorOrder: [7, 0, 0, 0, 0, 2, 4, 0, 0, 0, 0, 9]
 * Cells in columnMajorOrder: [7, 0, 0, 0, 2, 0, 0, 4, 0, 0, 0, 9]
 */

TEST_F(RegularGridMapFullIterTest, iterAll_rowMajor1) {
  std::vector<int> expectedCount = {7, -1, -1, -1, -1, 2, 4, -1, -1, -1, -1, 9};

  auto iter = g1->iter("ymf", true);
  for (; iter.getIndex() < iter.getMaxIndex(); iter++) {
    EXPECT_EQ(expectedCount[iter.getIndex()], (*iter)->getCount());
  }
}

TEST_F(RegularGridMapFullIterTest, iterAll_rowMajor2) {
  std::vector<int> expectedCount = {7, -1, -1, -1, -1, 2, 4, -1, -1, -1, -1, 9};

  auto iter = g1->iter("ymf", true);
  for (; iter != iter.end(); iter++) {
    EXPECT_EQ(expectedCount[iter.getIndex()], (*iter)->getCount());
  }
}

TEST_F(RegularGridMapFullIterTest, iterAll_rowMajor3) {
  std::vector<int> expectedCount = {7, -1, -1, -1, -1, 2, 4, -1, -1, -1, -1, 9};

  int i = 0;
  auto iter = g1->iter("ymf", true);
  for (const auto& e : iter) {
    EXPECT_EQ(expectedCount[i], e->getCount());
    i++;
  }
}

TEST_F(RegularGridMapFullIterTest, iterAll_columnMajor1) {
  std::vector<int> expectedCount = {7, -1, -1, -1, 2, -1, -1, 4, -1, -1, -1, 9};

  int i = 0;
  auto iter = g1->iter("ymf", true);
  for (const auto& e : iter) {
    EXPECT_EQ(expectedCount[i], e->getCount());
    i++;
  }
}
