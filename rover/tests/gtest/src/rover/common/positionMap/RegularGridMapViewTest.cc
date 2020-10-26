/*
 * GripMapViewTest.cc
 *
 *  Created on: Oct 8, 2020
 *      Author: sts
 */

#include <gtest/gtest.h>
#include <boost/range/algorithm.hpp>
#include <memory>
#include <string>
#include "rover/common/positionMap/RegularGridMap.h"

using namespace rover;

using StrEntry = IEntry<std::string, omnetpp::simtime_t>;
using StrLocalEntry = IEntry<std::string, omnetpp::simtime_t>;
using CellStrEntry = CellEntry<StrEntry>;

class RegularGridMapViewTest : public ::testing::Test {
  // Test fixture with 6 Nodes in 2 Cells as measured by the local node.
 protected:
  void SetUp() override {
    g1 = std::make_shared<RegularGridMap<std::string>>("Id1", 5.0,
                                                       std::make_pair(10, 10));
    // cell(2,5) local measure: (4, 3.2, 3.2)
    g1->incrementLocalOwnPos(coord2cell2_5, 3.2);
    g1->incrementLocal(coord1cell2_5, "NodeA", 3.2);
    g1->incrementLocal(coord1cell2_5, "NodeB", 3.2);
    g1->incrementLocal(coord1cell2_5, "NodeC", 3.2);
    // cell(1,1): local measure: (2, 3.1, 3.1)
    g1->incrementLocal(coord3cell1_1, "NodeD", 3.1);
    g1->incrementLocal(coord3cell1_1, "NodeF", 3.1);

    // data received from NodeE with NO local information
    g1->update(std::make_pair(3, 3), "NodeE", this->m(9, 5.3, 5.27));
    g1->update(std::make_pair(5, 5), "NodeE", this->m(7, 5.3, 5.07));
  }

  std::shared_ptr<StrEntry> m(int count, double time1, double time2) {
    return std::make_shared<StrEntry>(count, time1, time2);
  }

  traci::TraCIPosition coord1cell2_5{13.0, 27.0};
  traci::TraCIPosition coord2cell2_5{13.5, 27.7};
  traci::TraCIPosition coord3cell1_1{5.5, 7.8};
  traci::TraCIPosition coord4cell_2_1{11.0, 7.8};
  std::shared_ptr<RegularGridMap<std::string>> g1;
};

TEST_F(RegularGridMapViewTest, range_getId) {
  auto view = g1->getView("local");
  EXPECT_STREQ(g1->getNodeId().c_str(), view->getId().c_str());
}

TEST_F(RegularGridMapViewTest, range_size) {
  auto view = g1->getView("local");

  // 2 cells should be found
  EXPECT_EQ(2, view->size());
}

TEST_F(RegularGridMapViewTest, hasValueTrue) {
  auto view = g1->getView("local");

  EXPECT_TRUE(view->hasValue(std::make_pair(2, 5)));
}

TEST_F(RegularGridMapViewTest, hasValueFalse) {
  auto view = g1->getView("local");

  EXPECT_FALSE(view->hasValue(std::make_pair(33, 32)));
}

TEST_F(RegularGridMapViewTest, range_localMap) {
  auto view = g1->getView("local");

  auto e1 = view->find(std::make_pair(2, 5))->second;
  EXPECT_TRUE(e1->valid());
  EXPECT_DOUBLE_EQ(3.2, e1->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.2, e1->getReceivedTime().dbl());
  EXPECT_EQ(4, e1->getCount());

  auto e2 = view->find(std::make_pair(1, 1))->second;
  EXPECT_TRUE(e2->valid());
  EXPECT_DOUBLE_EQ(3.1, e2->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.1, e2->getReceivedTime().dbl());
  EXPECT_EQ(2, e2->getCount());
}

TEST_F(RegularGridMapViewTest, range_localMapValidOnly) {
  // reset cell (2,5) this cell must not be in the view
  g1->getCellEntry(std::make_pair(2, 5)).reset();
  auto view = g1->getView("local");

  // 2 cells should be found
  EXPECT_EQ(1, view->size());

  auto e2 = view->find(std::make_pair(1, 1))->second;
  EXPECT_TRUE(e2->valid());
  EXPECT_DOUBLE_EQ(3.1, e2->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.1, e2->getReceivedTime().dbl());
  EXPECT_EQ(2, e2->getCount());
}

TEST_F(RegularGridMapViewTest, range_youngestMeasureMap) {
  auto view = g1->getView("ymf");

  // 2 cells should be found
  EXPECT_EQ(4, view->size());

  auto e1 = view->find(std::make_pair(2, 5))->second;
  EXPECT_TRUE(e1->valid());
  EXPECT_DOUBLE_EQ(3.2, e1->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.2, e1->getReceivedTime().dbl());
  EXPECT_EQ(4, e1->getCount());

  e1 = view->find(std::make_pair(1, 1))->second;
  EXPECT_TRUE(e1->valid());
  EXPECT_DOUBLE_EQ(3.1, e1->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.1, e1->getReceivedTime().dbl());
  EXPECT_EQ(2, e1->getCount());

  e1 = view->find(std::make_pair(3, 3))->second;
  EXPECT_TRUE(e1->valid());
  EXPECT_DOUBLE_EQ(5.3, e1->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(5.27, e1->getReceivedTime().dbl());
  EXPECT_EQ(9, e1->getCount());

  e1 = view->find(std::make_pair(5, 5))->second;
  EXPECT_TRUE(e1->valid());
  EXPECT_DOUBLE_EQ(5.3, e1->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(5.07, e1->getReceivedTime().dbl());
  EXPECT_EQ(7, e1->getCount());
}

TEST_F(RegularGridMapViewTest, range_youngestMeasureOverrideLocalMap1) {
  // measure time 5.3 > 3.2 --> use this one in any case
  g1->update(std::make_pair(1, 1), "NodeE", this->m(9, 5.3, 5.27));

  auto view = g1->getView("ymf");
  auto e1 = view->find(std::make_pair(1, 1))->second;
  EXPECT_TRUE(e1->valid());
  EXPECT_DOUBLE_EQ(5.3, e1->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(5.27, e1->getReceivedTime().dbl());
  EXPECT_EQ(9, e1->getCount());
}

TEST_F(RegularGridMapViewTest, range_youngestMeasureOverrideLocalMap2) {
  // measure time 1.3 < 3.2 --> new measurement is older than existing local
  // measure use local measure
  g1->update(std::make_pair(1, 1), "NodeE", this->m(9, 1.3, 5.27));

  auto view = g1->getView("ymf");
  auto e1 = view->find(std::make_pair(1, 1))->second;
  EXPECT_TRUE(e1->valid());
  EXPECT_DOUBLE_EQ(3.1, e1->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(3.1, e1->getReceivedTime().dbl());
  EXPECT_EQ(2, e1->getCount());
}

TEST_F(RegularGridMapViewTest, range_youngestMeasureOverrideLocalMap3) {
  // measure time 5.3 > 3.2 --> use NodeE one in any case
  g1->update(std::make_pair(1, 1), "NodeE", this->m(9, 5.3, 5.27));
  g1->update(std::make_pair(1, 1), "NodeX", this->m(9, 1.3, 5.27));

  // cell (1,1) should contain 3 measurements
  int numMeasurements = g1->getCellEntry(std::make_pair(1, 1)).size();
  EXPECT_EQ(3, numMeasurements);

  auto view = g1->getView("ymf");
  auto e1 = view->find(std::make_pair(1, 1))->second;
  EXPECT_TRUE(e1->valid());
  EXPECT_DOUBLE_EQ(5.3, e1->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(5.27, e1->getReceivedTime().dbl());
  EXPECT_EQ(9, e1->getCount());
}

TEST_F(RegularGridMapViewTest, range_checkLazyEval) {
  // views are based on boost ranges. The range only consists of iterators and
  // is reevaluated each time the range is accessed.
  auto view = g1->getView("ymf");
  EXPECT_EQ(4, view->size());
  g1->update(std::make_pair(4, 4), "NodeE", this->m(9, 5.3, 5.27));
  g1->update(std::make_pair(6, 9), "NodeX", this->m(9, 1.3, 5.27));
  EXPECT_EQ(6, view->size());
}

/*
 * ***************************************************************************
 */

class RegularGridMapSourceNodeViewTest : public ::testing::Test {
 protected:
  void SetUp() override {
    g1 = std::make_shared<RegularGridMap<std::string>>("NodeA", 5.0,
                                                       std::make_pair(10, 10));

    // cell(2,5) local measure: (3, 3.2, 3.2)
    g1->incrementLocalOwnPos(coord2cell2_5, 3.2);
    g1->incrementLocal(coord1cell2_5, "NodeB", 3.2);
    g1->incrementLocal(coord1cell2_5, "NodeC", 3.2);
    // cell(1,1): local measure: (2, 3.1, 3.1)
    g1->incrementLocal(coord3cell1_1, "NodeD", 3.1);
    g1->incrementLocal(coord3cell1_1, "NodeF", 3.1);
  }

  std::shared_ptr<StrEntry> m(int count, double time1, double time2) {
    return std::make_shared<StrEntry>(count, time1, time2);
  }

  traci::TraCIPosition coord1cell2_5{13.0, 27.0};
  traci::TraCIPosition coord2cell2_5{13.5, 27.7};
  traci::TraCIPosition coord3cell1_1{5.5, 7.8};
  std::shared_ptr<RegularGridMap<std::string>> g1;
};

TEST_F(RegularGridMapSourceNodeViewTest, getKey_columnMajorOrder) {
  // received from NodeE
  g1->update(std::make_pair(3, 3), "NodeE", this->m(9, 5.3, 5.27));
  g1->update(std::make_pair(5, 5), "NodeE", this->m(7, 5.3, 5.07));

  // BySource View only contains these two nodes.
  auto view = g1->getViewBySource("NodeE");
  EXPECT_EQ(2, view->size());
  auto m = view->find(std::make_pair(3, 3))->second;
  EXPECT_TRUE(m->valid());
  EXPECT_EQ(9, m->getCount());

  m = view->find(std::make_pair(5, 5))->second;
  EXPECT_TRUE(m->valid());
  EXPECT_EQ(7, m->getCount());
}

// SourceNodeView
