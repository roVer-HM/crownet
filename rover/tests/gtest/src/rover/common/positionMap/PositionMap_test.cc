/*
 * RegularGridMap_test.cc
 *
 *  Created on: Sep 15, 2020
 *      Author: sts
 */

#include <gtest/gtest.h>
#include <boost/range/algorithm.hpp>
#include <memory>
#include <string>
#include "rover/common/positionMap/DensityMeasure.h"
#include "rover/common/positionMap/RegularGridMap.h"

using namespace rover;

class RegularGridMapIncrementLocalTest : public ::testing::Test {
 protected:
  void SetUp() override {
    g1 = std::make_shared<RegularGridMap<std::pair<int, int>, std::string>>(
        "Id1", 5.0);
  }

  inet::Coord coord1cell2_5{13.0, 27.0};
  inet::Coord coord2cell2_5{13.5, 27.7};
  inet::Coord coord3cell1_1{5.5, 7.8};
  std::shared_ptr<RegularGridMap<std::pair<int, int>, std::string>> g1;
};

TEST_F(RegularGridMapIncrementLocalTest, getNodeId) {
  std::string intendend("Id1");
  EXPECT_STREQ(intendend.c_str(), g1->getNodeId().c_str());
}

TEST_F(RegularGridMapIncrementLocalTest, getGridSize) {
  EXPECT_EQ(5.0, g1->getGridSize());
}

TEST_F(RegularGridMapIncrementLocalTest, incrementLocal1) {
  // must be in cell(2, 5)
  g1->incrementLocal(coord1cell2_5, "Node5", 3.3);

  auto entry = g1->getCellEntry(std::make_pair(2, 5)).getLocal();
  EXPECT_TRUE(entry->valid());
  EXPECT_DOUBLE_EQ(3.3, entry->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.3, entry->getReceivedTime().dbl());
  EXPECT_EQ(1, entry->getCount());
}

TEST_F(RegularGridMapIncrementLocalTest,
       incrementLocal2_addMultipleNodesSameCell) {
  // must be in cell(2, 5)
  g1->incrementLocal(coord1cell2_5, "Node5", 3.3);

  auto entry = g1->getCellEntry(std::make_pair(2, 5)).getLocal();
  EXPECT_TRUE(entry->valid());
  EXPECT_DOUBLE_EQ(3.3, entry->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.3, entry->getReceivedTime().dbl());
  EXPECT_EQ(1, entry->getCount());

  // add second Node to same cell
  g1->incrementLocal(coord1cell2_5, "Node4", 3.7);
  // same cell so use entry
  EXPECT_TRUE(entry->valid());
  EXPECT_DOUBLE_EQ(3.7, entry->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.7, entry->getReceivedTime().dbl());
  EXPECT_EQ(2, entry->getCount());

  auto locMeasure =
      std::static_pointer_cast<LocalDensityMeasure<std::string>>(entry);
  auto idSet = locMeasure->nodeIds;
  EXPECT_TRUE(idSet.find("Node5") != idSet.end());
  EXPECT_TRUE(idSet.find("Node4") != idSet.end());
}

TEST_F(RegularGridMapIncrementLocalTest,
       incrementLocal2_addMultipleNodesDifferentCell) {
  // must be in cell(2, 5)
  g1->incrementLocal(coord1cell2_5, "Id1", 3.3);

  auto entry = g1->getCellEntry(std::make_pair(2, 5)).getLocal();
  EXPECT_TRUE(entry->valid());
  EXPECT_DOUBLE_EQ(3.3, entry->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.3, entry->getReceivedTime().dbl());
  EXPECT_EQ(1, entry->getCount());

  // add second Node to different cell
  g1->incrementLocal(coord3cell1_1, "Node4", 3.7);
  auto entry2 = g1->getCellEntry(std::make_pair(1, 1)).getLocal();
  EXPECT_TRUE(entry2->valid());
  EXPECT_DOUBLE_EQ(3.7, entry2->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.7, entry2->getReceivedTime().dbl());
  EXPECT_EQ(1, entry2->getCount());
  // old cell must not change
  EXPECT_TRUE(entry->valid());
  EXPECT_DOUBLE_EQ(3.3, entry->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.3, entry->getReceivedTime().dbl());
  EXPECT_EQ(1, entry->getCount());
}

TEST_F(RegularGridMapIncrementLocalTest, incrementLocal3_ownPosition) {
  EXPECT_EQ(std::make_pair(0, 0), g1->getCellId());

  // must be in cell(2, 5)
  g1->incrementLocalOwnPos(coord1cell2_5, 3.3);

  auto entry = g1->getCellEntry(std::make_pair(2, 5)).getLocal();
  EXPECT_TRUE(entry->valid());
  EXPECT_DOUBLE_EQ(3.3, entry->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.3, entry->getReceivedTime().dbl());
  EXPECT_EQ(1, entry->getCount());
  EXPECT_EQ(std::make_pair(2, 5), g1->getCellId());
}

TEST_F(RegularGridMapIncrementLocalTest,
       incrementLocal_duplicateNode_sameCell) {
  // add node multiple times [err]
  g1->incrementLocal(coord1cell2_5, "Node5", 3.3);
  try {
    g1->incrementLocal(coord2cell2_5, "Node5", 3.3);
    FAIL() << "adding same node multiple times must fail";
  } catch (omnetpp::cRuntimeError& e) {
  }
}

// TODO: Fix
// TEST_F(RegularGridMapIncrementLocalTest,
//       incrementLocal_duplicateNode_differentCell) {
//  // add node multiple times [err]
//  g1->incrementLocal(coord1cell2_5, "Node5", 3.3);
//  try {
//    // same node different cell
//    g1->incrementLocal(coord3cell1_1, "Node5", 3.3);
//    FAIL() << "adding same node multiple times in different cells must fail";
//  } catch (omnetpp::cRuntimeError& e) {
//  }
//}

TEST_F(RegularGridMapIncrementLocalTest, incrementLocal2_resetLocalMap) {
  // must be in cell(2, 5)
  g1->incrementLocal(coord1cell2_5, "Id1", 3.3);
  // add second Node to different cell (1,1)
  g1->incrementLocal(coord3cell1_1, "Node4", 3.7);
  auto entry1 = g1->getCellEntry(std::make_pair(2, 5)).getLocal();
  auto entry2 = g1->getCellEntry(std::make_pair(1, 1)).getLocal();
  EXPECT_TRUE(entry1->valid());
  EXPECT_TRUE(entry2->valid());
  g1->resetLocalMap();
  EXPECT_FALSE(entry1->valid());
  EXPECT_FALSE(entry2->valid());
  EXPECT_EQ(0, entry1->getCount());
  EXPECT_EQ(0, entry2->getCount());
}

class RegularGridMapUpdateTest : public RegularGridMapIncrementLocalTest {
  // Test fixture with 4 Nodes in 1 Cells as measured by the local node.
 protected:
  void SetUp() override {
    RegularGridMapIncrementLocalTest::SetUp();
    // cell(2,5)
    g1->incrementLocalOwnPos(coord2cell2_5, 3.2);
    g1->incrementLocal(coord1cell2_5, "NodeA", 3.2);
    g1->incrementLocal(coord1cell2_5, "NodeB", 3.2);
    g1->incrementLocal(coord1cell2_5, "NodeC", 3.2);
  }
};

TEST_F(RegularGridMapUpdateTest, getLocal_valid) {
  // valid local measure for cell (2,5)
  auto entry = g1->getCellEntry(std::make_pair(2, 5));
  EXPECT_TRUE(entry.getLocal()->valid());
}

TEST_F(RegularGridMapUpdateTest, getLocal_invalid) {
  // valid local measure for cell (2,5)
  auto entry = g1->getCellEntry(std::make_pair(3, 3));
  EXPECT_FALSE(entry.getLocal()->valid());
}

TEST_F(RegularGridMapUpdateTest, update1_newCell) {
  auto _m1 = std::make_shared<DensityMeasure<std::string>>(5, 3.54, 3.50);
  g1->update(std::make_pair(3, 3), "NodeE", std::move(_m1));
  auto measure = g1->getCellEntry(std::make_pair(3, 3)).youngestMeasureFirst();
  EXPECT_TRUE(measure->valid());
  EXPECT_DOUBLE_EQ(3.54, measure->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.50, measure->getReceivedTime().dbl());
  EXPECT_EQ(5, measure->getCount());
}

TEST_F(RegularGridMapUpdateTest, update2_withOlderLocalMeasure1) {
  auto _m1 = std::make_shared<DensityMeasure<std::string>>(5, 3.54, 3.50);
  g1->update(std::make_pair(2, 5), "NodeE", std::move(_m1));
  auto measure = g1->getCellEntry(std::make_pair(2, 5)).youngestMeasureFirst();
  // expect value from NodeE (5, 3.54, 3.50)
  EXPECT_TRUE(measure->valid());
  EXPECT_DOUBLE_EQ(3.54, measure->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.50, measure->getReceivedTime().dbl());
  EXPECT_EQ(5, measure->getCount());
}

TEST_F(RegularGridMapUpdateTest, update3_withYoungerLocalMeasure1) {
  auto _m1 = std::make_shared<DensityMeasure<std::string>>(5, 1.54, 3.50);
  g1->update(std::make_pair(2, 5), "NodeE", std::move(_m1));
  auto measure = g1->getCellEntry(std::make_pair(2, 5)).youngestMeasureFirst();
  // expect value from Local node (4, 3.2, 3.2)
  EXPECT_TRUE(measure->valid());
  EXPECT_DOUBLE_EQ(3.2, measure->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.2, measure->getReceivedTime().dbl());
  EXPECT_EQ(4, measure->getCount());
}

class RegularGridMapRangeTest : public RegularGridMapIncrementLocalTest {
  // Test fixture with 6 Nodes in 2 Cells as measured by the local node.
 protected:
  void SetUp() override {
    RegularGridMapIncrementLocalTest::SetUp();
    // cell(2,5) local measure: (4, 3.2, 3.2)
    g1->incrementLocalOwnPos(coord2cell2_5, 3.2);
    g1->incrementLocal(coord1cell2_5, "NodeA", 3.2);
    g1->incrementLocal(coord1cell2_5, "NodeB", 3.2);
    g1->incrementLocal(coord1cell2_5, "NodeC", 3.2);
    // cell(1,1): local measure: (2, 3.1, 3.1)
    g1->incrementLocal(coord3cell1_1, "NodeD", 3.1);
    g1->incrementLocal(coord3cell1_1, "NodeC", 3.1);

    // data received from NodeE with NO local information
    g1->update(std::make_pair(3, 3), "NodeE", std::move(this->m(9, 5.3, 5.27)));
    g1->update(std::make_pair(5, 5), "NodeE", std::move(this->m(7, 5.3, 5.07)));
  }

  std::shared_ptr<DensityMeasure<std::string>> m(int count, double time1,
                                                 double time2) {
    return std::make_shared<DensityMeasure<std::string>>(count, time1, time2);
  }

  inet::Coord coord4cell_2_1{11.0, 7.8};
};

TEST_F(RegularGridMapRangeTest, range_getId) {
  auto view = g1->getView("local");
  EXPECT_STREQ(g1->getNodeId().c_str(), view->getId().c_str());
}

TEST_F(RegularGridMapRangeTest, range_size) {
  auto view = g1->getView("local");

  // 2 cells should be found
  EXPECT_EQ(2, view->size());
}

TEST_F(RegularGridMapRangeTest, range_localMap) {
  auto view = g1->getView("local");

  auto e1 = view->getValue(std::make_pair(2, 5));
  EXPECT_TRUE(e1->valid());
  EXPECT_DOUBLE_EQ(3.2, e1->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.2, e1->getReceivedTime().dbl());
  EXPECT_EQ(4, e1->getCount());

  auto e2 = view->getValue(std::make_pair(1, 1));
  EXPECT_TRUE(e2->valid());
  EXPECT_DOUBLE_EQ(3.1, e2->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.1, e2->getReceivedTime().dbl());
  EXPECT_EQ(2, e2->getCount());
}

TEST_F(RegularGridMapRangeTest, range_localMapValidOnly) {
  // reset cell (2,5) this cell must not be in the view
  g1->getCellEntry(std::make_pair(2, 5)).reset();
  auto view = g1->getView("local");

  // 2 cells should be found
  EXPECT_EQ(1, view->size());

  auto e2 = view->getValue(std::make_pair(1, 1));
  EXPECT_TRUE(e2->valid());
  EXPECT_DOUBLE_EQ(3.1, e2->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.1, e2->getReceivedTime().dbl());
  EXPECT_EQ(2, e2->getCount());
}

TEST_F(RegularGridMapRangeTest, range_youngestMeasureMap) {
  auto view = g1->getView("ymf");

  // 2 cells should be found
  EXPECT_EQ(4, view->size());

  auto e1 = view->getValue(std::make_pair(2, 5));
  EXPECT_TRUE(e1->valid());
  EXPECT_DOUBLE_EQ(3.2, e1->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.2, e1->getReceivedTime().dbl());
  EXPECT_EQ(4, e1->getCount());

  e1 = view->getValue(std::make_pair(1, 1));
  EXPECT_TRUE(e1->valid());
  EXPECT_DOUBLE_EQ(3.1, e1->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.1, e1->getReceivedTime().dbl());
  EXPECT_EQ(2, e1->getCount());

  e1 = view->getValue(std::make_pair(3, 3));
  EXPECT_TRUE(e1->valid());
  EXPECT_DOUBLE_EQ(5.3, e1->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(5.27, e1->getReceivedTime().dbl());
  EXPECT_EQ(9, e1->getCount());

  e1 = view->getValue(std::make_pair(5, 5));
  EXPECT_TRUE(e1->valid());
  EXPECT_DOUBLE_EQ(5.3, e1->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(5.07, e1->getReceivedTime().dbl());
  EXPECT_EQ(7, e1->getCount());
}

TEST_F(RegularGridMapRangeTest, range_youngestMeasureOverrideLocalMap1) {
  // measure time 5.3 > 3.2 --> use this one in any case
  g1->update(std::make_pair(1, 1), "NodeE", std::move(this->m(9, 5.3, 5.27)));

  auto view = g1->getView("ymf");
  auto e1 = view->getValue(std::make_pair(1, 1));
  EXPECT_TRUE(e1->valid());
  EXPECT_DOUBLE_EQ(5.3, e1->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(5.27, e1->getReceivedTime().dbl());
  EXPECT_EQ(9, e1->getCount());
}

TEST_F(RegularGridMapRangeTest, range_youngestMeasureOverrideLocalMap2) {
  // measure time 1.3 < 3.2 --> new measurement is older than existing local
  // measure use local measure
  g1->update(std::make_pair(1, 1), "NodeE", std::move(this->m(9, 1.3, 5.27)));

  auto view = g1->getView("ymf");
  auto e1 = view->getValue(std::make_pair(1, 1));
  EXPECT_TRUE(e1->valid());
  EXPECT_DOUBLE_EQ(3.1, e1->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.1, e1->getReceivedTime().dbl());
  EXPECT_EQ(2, e1->getCount());
}

TEST_F(RegularGridMapRangeTest, range_youngestMeasureOverrideLocalMap3) {
  // measure time 5.3 > 3.2 --> use NodeE one in any case
  g1->update(std::make_pair(1, 1), "NodeE", std::move(this->m(9, 5.3, 5.27)));
  g1->update(std::make_pair(1, 1), "NodeX", std::move(this->m(9, 1.3, 5.27)));

  // cell (1,1) should contain 3 measurements
  int numMeasurements = g1->getCellEntry(std::make_pair(1, 1)).size();
  EXPECT_EQ(3, numMeasurements);

  auto view = g1->getView("ymf");
  auto e1 = view->getValue(std::make_pair(1, 1));
  EXPECT_TRUE(e1->valid());
  EXPECT_DOUBLE_EQ(5.3, e1->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(5.27, e1->getReceivedTime().dbl());
  EXPECT_EQ(9, e1->getCount());
}

TEST_F(RegularGridMapRangeTest, range_checkLazyEval) {
  // views are based on boost ranges. The range only consists of iterators and
  // is reevaluated each time the range is accessed.
  auto view = g1->getView("ymf");
  EXPECT_EQ(4, view->size());
  g1->update(std::make_pair(4, 4), "NodeE", std::move(this->m(9, 5.3, 5.27)));
  g1->update(std::make_pair(6, 9), "NodeX", std::move(this->m(9, 1.3, 5.27)));
  EXPECT_EQ(6, view->size());
}
