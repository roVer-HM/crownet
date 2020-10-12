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

class RegularGridMapIncrLocalTest : public ::testing::Test {
 protected:
  void SetUp() override {
    g1 = std::make_shared<RegularGridMap<std::string>>("Id1", 5.0,
                                                       this->cellKey(10, 10));
  }

  std::shared_ptr<DensityMeasure<std::string>> m(int count, double time1,
                                                 double time2) {
    return std::make_shared<DensityMeasure<std::string>>(count, time1, time2);
  }

  std::pair<int, int> cellKey(int x, int y) { return std::make_pair(x, y); }

  traci::TraCIPosition coord1cell2_5{13.0, 27.0};
  traci::TraCIPosition coord2cell2_5{13.5, 27.7};
  traci::TraCIPosition coord3cell1_1{5.5, 7.8};
  std::shared_ptr<RegularGridMap<std::string>> g1;
};

TEST_F(RegularGridMapIncrLocalTest, getNodeId) {
  std::string intendend("Id1");
  EXPECT_STREQ(intendend.c_str(), g1->getNodeId().c_str());
}

TEST_F(RegularGridMapIncrLocalTest, getGridSize) {
  EXPECT_EQ(5.0, g1->getGridSize());
}

TEST_F(RegularGridMapIncrLocalTest, incrementLocal1) {
  // must be in cell(2, 5)
  g1->incrementLocal(coord1cell2_5, "Node5", 3.3);

  auto entry = g1->getCellEntry(this->cellKey(2, 5)).getLocal();
  EXPECT_TRUE(entry->valid());
  EXPECT_DOUBLE_EQ(3.3, entry->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.3, entry->getReceivedTime().dbl());
  EXPECT_EQ(1, entry->getCount());
}

TEST_F(RegularGridMapIncrLocalTest, incrementLocal2_addMultipleNodesSameCell) {
  // must be in cell(2, 5)
  g1->incrementLocal(coord1cell2_5, "Node5", 3.3);

  auto entry = g1->getCellEntry(this->cellKey(2, 5)).getLocal();
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

TEST_F(RegularGridMapIncrLocalTest,
       incrementLocal2_addMultipleNodesDifferentCell) {
  // must be in cell(2, 5)
  g1->incrementLocal(coord1cell2_5, "Id1", 3.3);

  auto entry = g1->getCellEntry(this->cellKey(2, 5)).getLocal();
  EXPECT_TRUE(entry->valid());
  EXPECT_DOUBLE_EQ(3.3, entry->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.3, entry->getReceivedTime().dbl());
  EXPECT_EQ(1, entry->getCount());

  // add second Node to different cell
  g1->incrementLocal(coord3cell1_1, "Node4", 3.7);
  auto entry2 = g1->getCellEntry(this->cellKey(1, 1)).getLocal();
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

TEST_F(RegularGridMapIncrLocalTest, incrementLocal3_ownPosition) {
  EXPECT_EQ(this->cellKey(0, 0), g1->getCellId());

  // must be in cell(2, 5)
  g1->incrementLocalOwnPos(coord1cell2_5, 3.3);

  auto entry = g1->getCellEntry(this->cellKey(2, 5)).getLocal();
  EXPECT_TRUE(entry->valid());
  EXPECT_DOUBLE_EQ(3.3, entry->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.3, entry->getReceivedTime().dbl());
  EXPECT_EQ(1, entry->getCount());
  EXPECT_EQ(this->cellKey(2, 5), g1->getCellId());
}

TEST_F(RegularGridMapIncrLocalTest, incrementLocal4_ownPositionCell) {}

TEST_F(RegularGridMapIncrLocalTest, incrementLocal_duplicateNode_sameCell) {
  // add node multiple times [err]
  g1->incrementLocal(coord1cell2_5, "Node5", 3.3);
  try {
    g1->incrementLocal(coord2cell2_5, "Node5", 3.3);
    FAIL() << "adding same node multiple times must fail";
  } catch (omnetpp::cRuntimeError& e) {
  }
}

TEST_F(RegularGridMapIncrLocalTest,
       incrementLocal_duplicateNode_differentCell) {
  // add node multiple times [err]
  g1->incrementLocal(coord1cell2_5, "Node5", 3.3);
  try {
    // same node different cell
    g1->incrementLocal(coord3cell1_1, "Node5", 3.3);
    FAIL() << "adding same node multiple times in different cells must fail";
  } catch (omnetpp::cRuntimeError& e) {
  }
}

/**
 * Check that after resetMap call the local measurements are invalidated.
 */
TEST_F(RegularGridMapIncrLocalTest, incrementLocal2_resetMap) {
  // must be in cell(2, 5)
  g1->incrementLocal(coord1cell2_5, "Id1", 3.3);
  // add second Node to different cell (1,1)
  g1->incrementLocal(coord3cell1_1, "Node4", 3.7);
  auto entry1 = g1->getCellEntry(this->cellKey(2, 5)).getLocal();
  auto entry2 = g1->getCellEntry(this->cellKey(1, 1)).getLocal();
  EXPECT_TRUE(entry1->valid());
  EXPECT_TRUE(entry2->valid());
  // Id1 must be in (2, 5)
  EXPECT_TRUE(g1->hasNeighbour("Id1", this->cellKey(2, 5)));
  EXPECT_TRUE(g1->hasNeighbour("Node4", this->cellKey(1, 1)));
  EXPECT_EQ(2, g1->neighbourCount());

  g1->resetMap();
  EXPECT_FALSE(entry1->valid());
  EXPECT_FALSE(entry2->valid());
  EXPECT_EQ(0, entry1->getCount());
  EXPECT_EQ(0, entry2->getCount());
  EXPECT_EQ(0, g1->neighbourCount());
}

TEST_F(RegularGridMapIncrLocalTest, incrementLocal2_resetMap2) {
  // must be in cell(2, 5)
  g1->incrementLocal(coord1cell2_5, "NodeB", 3.3);
  // add second Node to different cell (1,1)
  g1->incrementLocal(coord1cell2_5, "NodeC", 3.7);
  auto entry1 = g1->getCellEntry(this->cellKey(2, 5)).getLocal();
  auto entry2 = g1->getCellEntry(this->cellKey(1, 1)).getLocal();
  EXPECT_TRUE(entry1->valid());
  g1->resetMap();
  EXPECT_FALSE(entry1->valid());
  EXPECT_FALSE(entry2->valid());
  EXPECT_EQ(0, entry1->getCount());
  EXPECT_EQ(0, entry2->getCount());
}

TEST_F(RegularGridMapIncrLocalTest, incrementLocal2_resetMapForNodeId) {
  // add local count to check that it is NOT changed by resetMap(...)
  g1->incrementLocal(coord1cell2_5, "Id1", 3.3);
  EXPECT_EQ(1, g1->getView("local")->size());

  auto measure1 = this->m(3, 1., 1.);
  auto measure2 = this->m(5, 1., 1.);
  g1->update(cellKey(2, 4), "Node_N", std::move(measure1));
  g1->update(cellKey(1, 2), "Node_N", std::move(measure2));

  auto viewN = g1->getViewBySource("Node_N");
  EXPECT_EQ(2, viewN->size());
  g1->resetMap("Node_N");
  EXPECT_EQ(0, viewN->size());

  // check locak count not effected by resetMap(...)
  EXPECT_EQ(1, g1->getView("local")->size());
}

TEST_F(RegularGridMapIncrLocalTest, incrementLocal3_reset) {
  // must be in cell(2, 5)
  g1->incrementLocal(coord1cell2_5, "Id1", 3.3);
  // add second Node to different cell (1,1)
  g1->incrementLocal(coord3cell1_1, "Node4", 3.7);
  auto entry1 = g1->getCellEntry(this->cellKey(2, 5)).getLocal();
  auto entry2 = g1->getCellEntry(this->cellKey(1, 1)).getLocal();
  EXPECT_TRUE(entry1->valid());
  EXPECT_TRUE(entry2->valid());
  g1->resetMap();
  EXPECT_FALSE(entry1->valid());
  EXPECT_FALSE(entry2->valid());
  EXPECT_EQ(0, entry1->getCount());
  EXPECT_EQ(0, entry2->getCount());
}

class RegularGridMapUpdateTest : public RegularGridMapIncrLocalTest {
  // Test fixture with 4 Nodes in 1 Cell as measured by the local node.
 protected:
  void SetUp() override {
    RegularGridMapIncrLocalTest::SetUp();
    // cell(2,5)
    g1->incrementLocalOwnPos(coord2cell2_5, 3.2);
    g1->incrementLocal(coord1cell2_5, "NodeA", 3.2);
    g1->incrementLocal(coord1cell2_5, "NodeB", 3.2);
    g1->incrementLocal(coord1cell2_5, "NodeC", 3.2);
  }
};

TEST_F(RegularGridMapUpdateTest, getLocal_valid) {
  // valid local measure for cell (2,5)
  auto entry = g1->getCellEntry(this->cellKey(2, 5));
  EXPECT_TRUE(entry.getLocal()->valid());
}

TEST_F(RegularGridMapUpdateTest, getLocal_invalid) {
  // valid local measure for cell (2,5)
  auto entry = g1->getCellEntry(this->cellKey(3, 3));
  EXPECT_FALSE(entry.getLocal()->valid());
}

TEST_F(RegularGridMapUpdateTest, update1_newCell) {
  auto _m1 = this->m(5, 3.54, 3.50);
  g1->update(this->cellKey(3, 3), "NodeE", std::move(_m1));
  auto measure = g1->getCellEntry(this->cellKey(3, 3)).youngestMeasureFirst();
  EXPECT_TRUE(measure->valid());
  EXPECT_DOUBLE_EQ(3.54, measure->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.50, measure->getReceivedTime().dbl());
  EXPECT_EQ(5, measure->getCount());
}

TEST_F(RegularGridMapUpdateTest, update2_withOlderLocalMeasure1) {
  auto _m1 = std::make_shared<DensityMeasure<std::string>>(5, 3.54, 3.50);
  g1->update(this->cellKey(2, 5), "NodeE", std::move(_m1));
  auto measure = g1->getCellEntry(this->cellKey(2, 5)).youngestMeasureFirst();
  // expect value from NodeE (5, 3.54, 3.50)
  EXPECT_TRUE(measure->valid());
  EXPECT_DOUBLE_EQ(3.54, measure->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.50, measure->getReceivedTime().dbl());
  EXPECT_EQ(5, measure->getCount());
}

TEST_F(RegularGridMapUpdateTest, update3_withYoungerLocalMeasure1) {
  auto _m1 = this->m(5, 1.54, 3.50);
  g1->update(this->cellKey(2, 5), "NodeE", std::move(_m1));
  auto measure = g1->getCellEntry(this->cellKey(2, 5)).youngestMeasureFirst();
  // expect value from Local node (4, 3.2, 3.2)
  EXPECT_TRUE(measure->valid());
  EXPECT_DOUBLE_EQ(3.2, measure->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.2, measure->getReceivedTime().dbl());
  EXPECT_EQ(4, measure->getCount());
}

TEST_F(RegularGridMapUpdateTest, moveNodeInLocalMap1_nodeExists) {
  // clear state
  g1->resetMap();
  g1->resetMap("NodeE");

  // NodeE is in cell (1,1)
  EXPECT_EQ(0, g1->getCellEntry(this->cellKey(1, 1)).getLocal()->getCount());
  g1->incrementLocal(coord3cell1_1, "NodeE", 2.0);
  EXPECT_EQ(1, g1->getCellEntry(this->cellKey(1, 1)).getLocal()->getCount());

  // NodeE moved to cell(2,5)
  g1->moveNodeInLocalMap(this->cellKey(2, 5), "NodeE");
  // old cell must be one less than before (if 0 it must still be valid)
  EXPECT_EQ(0, g1->getCellEntry(this->cellKey(1, 1)).getLocal()->getCount());
  EXPECT_TRUE(g1->getCellEntry(this->cellKey(1, 1)).getLocal()->valid());
  // new cell must have one count more
  EXPECT_EQ(1, g1->getCellEntry(this->cellKey(2, 5)).getLocal()->getCount());
  EXPECT_TRUE(g1->getCellEntry(this->cellKey(2, 5)).getLocal()->valid());
}

TEST_F(RegularGridMapUpdateTest, moveNodeInLocalMap2_nodeNotExists) {
  // clear state
  g1->resetMap();
  g1->resetMap("NodeE");

  // no Node in cell(2,5)
  EXPECT_EQ(0, g1->getCellEntry(this->cellKey(2, 5)).getLocal()->getCount());

  // NodeE is new cell(2,5)
  g1->moveNodeInLocalMap(this->cellKey(2, 5), "NodeE");
  // cell must have one count more
  EXPECT_EQ(1, g1->getCellEntry(this->cellKey(2, 5)).getLocal()->getCount());
  EXPECT_TRUE(g1->getCellEntry(this->cellKey(2, 5)).getLocal()->valid());
}
