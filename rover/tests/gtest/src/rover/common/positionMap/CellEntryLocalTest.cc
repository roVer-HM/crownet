/*
 * CellEntryTest.cc
 *
 *  Created on: Oct 8, 2020
 *      Author: sts
 */

#include <gtest/gtest.h>
#include <string>
#include "rover/common/positionMap/Entry.h"
#include "rover/common/positionMap/PositionMap.h"

using namespace rover;

using IntEntry = IEntry<int, omnetpp::simtime_t>;
using CellIntEntry = CellEntry<IntEntry>;

class CellEntryLocalTest : public ::testing::Test {
 protected:
  void SetUp() override { cell = std::make_shared<CellIntEntry>(1); }

  std::shared_ptr<CellIntEntry> cell;
};

TEST_F(CellEntryLocalTest, localKey) { EXPECT_EQ(1, cell->localKey()); }

TEST_F(CellEntryLocalTest, hasLocalMeasure_1) {
  EXPECT_FALSE(cell->hasLocalMeasure());
  EXPECT_FALSE(
      cell->hasValid());  // local measure is a measure thus must count here.
}

TEST_F(CellEntryLocalTest, hasLocalMeasure_2) {
  cell->getLocal()->incrementCount(2.2);
  EXPECT_TRUE(cell->hasLocalMeasure());
  EXPECT_TRUE(
      cell->hasValid());  // local measure is a measure thus must count here.
}

TEST_F(CellEntryLocalTest, hasLocalValidMeasure_1) {
  // no local measure
  EXPECT_FALSE(cell->hasValidLocalMeasure());
  // has local measure but invalid
  cell->getLocal()->incrementCount(2.2);
  cell->resetLocalMeasure();
  EXPECT_FALSE(cell->hasValidLocalMeasure());
}

TEST_F(CellEntryLocalTest, hasLocalValidMeasure_2) {
  cell->getLocal()->incrementCount(2.2);
  EXPECT_TRUE(cell->hasValidLocalMeasure());
}

TEST_F(CellEntryLocalTest, local_increment_and_rest_1) {
  cell->getLocal()->incrementCount(2.2);
  EXPECT_EQ(1, cell->getLocal()->getCount());
  cell->getLocal()->incrementCount(2.2);
  cell->getLocal()->incrementCount(2.2);
  EXPECT_EQ(3, cell->getLocal()->getCount());
}

TEST_F(CellEntryLocalTest, local_increment_and_rest_2) {
  cell->getLocal()->incrementCount(2.2);
  EXPECT_EQ(1, cell->getLocal()->getCount());
  cell->getLocal()->incrementCount(2.2);
  cell->getLocal()->incrementCount(2.2);
  EXPECT_EQ(3, cell->getLocal()->getCount());
  // reset must start count form beginning.
  cell->resetLocalMeasure();
  cell->getLocal()->incrementCount(2.2);
  EXPECT_EQ(1, cell->getLocal()->getCount());
}

class CellEntryTest : public ::testing::Test {
 protected:
  void SetUp() override {
    cell = std::make_shared<CellIntEntry>(1);

    // local count 3
    cell->getLocal()->incrementCount(1.1);
    cell->getLocal()->incrementCount(1.1);
    cell->getLocal()->incrementCount(1.1);

    // node_id 3 count 2
    auto m = cell->get(node_id_3);
    m->incrementCount(3.3);
    m->incrementCount(3.3);

    // node_id 4 count 1 (invalid)
    m = cell->get(node_id_4);
    m->incrementCount(3.3);
    m->reset();
  }

  int node_id_3 = 3;
  int node_id_4 = 4;
  std::shared_ptr<CellIntEntry> cell;
};

TEST_F(CellEntryTest, size_1) { EXPECT_EQ(2, cell->size()); }

TEST_F(CellEntryTest, size_2) {
  cell->resetLocalMeasure();
  EXPECT_EQ(1, cell->size());
}

TEST_F(CellEntryTest, size_3) {
  cell->reset(node_id_3);
  EXPECT_EQ(1, cell->size());
}

TEST_F(CellEntryTest, size_4) {
  cell->reset();
  EXPECT_EQ(0, cell->size());
}

TEST_F(CellEntryTest, youngestMeasureFirst_1) {
  // is value from node_3, (count 2)
  auto m = cell->youngestMeasureFirst();
  EXPECT_DOUBLE_EQ(3.3, m->getMeasureTime().dbl());
  EXPECT_EQ(2, m->getCount());
}

TEST_F(CellEntryTest, youngestMeasureFirst_2) {
  // is now the local value. (count 4)
  cell->getLocal()->incrementCount(3.4);
  auto m = cell->youngestMeasureFirst();
  EXPECT_DOUBLE_EQ(3.4, m->getMeasureTime().dbl());
  EXPECT_EQ(4, m->getCount());
}

TEST_F(CellEntryTest, youngestMeasureFirst_3) {
  // in case of same time use local. (count 4)
  cell->getLocal()->incrementCount(3.3);
  auto m = cell->youngestMeasureFirst();
  EXPECT_DOUBLE_EQ(3.3, m->getMeasureTime().dbl());
  EXPECT_EQ(4, m->getCount());
}

TEST_F(CellEntryTest, hasMeasure_1) {
  EXPECT_TRUE(cell->hasMeasure(node_id_3));
  EXPECT_TRUE(cell->hasMeasure(node_id_4));
}

TEST_F(CellEntryTest, hasMeasure_2) { EXPECT_FALSE(cell->hasMeasure(42)); }

TEST_F(CellEntryTest, hasValidMeasure_1) {
  EXPECT_TRUE(cell->hasValidMeasure(node_id_3));
}

TEST_F(CellEntryTest, hasValidMeasure_2) {
  EXPECT_FALSE(cell->hasValidMeasure(node_id_4));
}

TEST_F(CellEntryTest, hasValidMeasure_3) {
  cell->reset();
  EXPECT_FALSE(cell->hasValidMeasure(node_id_3));
  EXPECT_FALSE(cell->hasValidMeasure(node_id_4));
  EXPECT_FALSE(cell->hasValidLocalMeasure());
}
