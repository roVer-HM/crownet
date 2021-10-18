/*
 * DcDMapIteratorTest.cc
 *
 *  Created on: Dec 1, 2020
 *      Author: sts
 */

#include "crownet/dcd/generic/iterator/DcDMapIterator.h"

#include <functional>
#include <memory>
#include <string>

#include "main_test.h"
#include "crownet/dcd/regularGrid/RegularCell.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"

using namespace crownet;

namespace {
RegularGridInfo  grid{{10.0, 10.0}, {1.0, 1.0}};
RegularDcdMapFactory dcdFactory{grid};
}

class DcDMapIteraotrTest : public BaseOppTest {
 public:
  using Entry = IEntry<IntIdentifer, omnetpp::simtime_t>;
  DcDMapIteraotrTest()
      : mapEmpty(dcdFactory.create(1)),
        mapLocal(dcdFactory.create(2)),
        mapFull(dcdFactory.create(3)) {}

  void incr(RegularDcdMap& map, double x, double y, int i, double t) {
    map.incrementLocal(traci::TraCIPosition(x, y), IntIdentifer(i), t);
  }
  void update(RegularDcdMap& map, int x, int y, int id, int count, double t) {
    auto e = std::make_shared<Entry>(count, t, t, IntIdentifer(id));
    map.update(GridCellID(x, y), std::move(e));
  }

  void SetUp() override {
    // [1, 1] count 2
    incr(mapLocal, 1.2, 1.2, 100, 30.0);
    incr(mapLocal, 1.5, 1.5, 101, 30.0);
    // [3, 3] count 3
    incr(mapLocal, 3.2, 3.2, 200, 32.0);
    incr(mapLocal, 3.5, 3.5, 201, 32.0);
    incr(mapLocal, 3.5, 3.5, 202, 32.0);
    // [4, 4] count 1
    incr(mapLocal, 4.2, 4.5, 300, 34.0);
    // local neighbors [ 100, 101, 200, 201, 202, 300] #6

    // [1, 1] Local = 2 / Ohter = 4
    incr(mapFull, 1.2, 1.2, 100, 30.0);
    incr(mapFull, 1.5, 1.5, 101, 30.0);
    update(mapFull, 1, 1, 800, 4, 31.0);
    // [3, 3] count 3 / Other = 3
    incr(mapFull, 3.2, 3.2, 200, 32.0);
    incr(mapFull, 3.5, 3.5, 201, 32.0);
    incr(mapFull, 3.5, 3.5, 202, 32.0);
    update(mapFull, 4, 4, 801, 3, 32.0);
    // [4, 4] count 2 / Other = 0
    incr(mapFull, 4.2, 4.5, 300, 34.0);
    incr(mapFull, 4.2, 4.5, 301, 34.0);
    // [6, 3] count 0 / Other = [5, 7, 9]
    update(mapFull, 6, 3, 803, 5, 33.0);
    update(mapFull, 6, 3, 805, 7, 31.0);
    update(mapFull, 6, 3, 808, 9, 39.0);
  }

 protected:
  RegularDcdMap mapEmpty;
  RegularDcdMap mapLocal;
  RegularDcdMap mapFull;
};

TEST_F(DcDMapIteraotrTest, iter_emptyMap) {
  EXPECT_TRUE(mapEmpty.begin() == mapEmpty.end());
  EXPECT_TRUE(mapEmpty.allLocal().begin() == mapEmpty.allLocal().end());
  EXPECT_TRUE(mapEmpty.validLocal().begin() == mapEmpty.validLocal().end());
}

TEST_F(DcDMapIteraotrTest, iter_notEmptyMap) {
  EXPECT_TRUE(mapLocal.begin() != mapLocal.end());
  EXPECT_TRUE(mapLocal.allLocal().begin() != mapLocal.allLocal().end());
  EXPECT_TRUE(mapLocal.validLocal().begin() != mapLocal.validLocal().end());
}

TEST_F(DcDMapIteraotrTest, iter_validLocal) {
  // validLocal returns no cells after reset nut allLocal still does.
  EXPECT_TRUE(mapLocal.validLocal().begin() != mapLocal.validLocal().end());
  EXPECT_TRUE(mapLocal.allLocal().begin() != mapLocal.allLocal().end());
  for (auto& m_val : mapLocal) {
    m_val.second.acceptSet(ResetVisitor(33.0));
  }
  EXPECT_TRUE(mapLocal.validLocal().begin() == mapLocal.validLocal().end());
  EXPECT_TRUE(mapLocal.allLocal().begin() != mapLocal.allLocal().end());
}

TEST_F(DcDMapIteraotrTest, iter_validLocal_2) {
  int i = 0;
  for (auto it = mapFull.validLocal(); it != it.end(); ++it) {
    auto mapVal = *it;
    EXPECT_TRUE(mapVal.second.hasValidLocal());
    ++i;
  }
  // not all cells have valid local measures
  EXPECT_LT(i, mapFull.getCells().size());
}

TEST_F(DcDMapIteraotrTest, iter_allLocal_2) {
  // invalidate one cell
  mapLocal.applyVisitorTo(GridCellID(1, 1), ResetVisitor(33.0));
  int valid = 0;
  for (auto it = mapLocal.validLocal(); it != it.end(); ++it) {
    auto mapVal = *it;
    EXPECT_TRUE(mapVal.second.hasValidLocal());
    ++valid;
  }
  int all = 0;
  for (auto it = mapLocal.allLocal(); it != it.end(); ++it) {
    auto mapVal = *it;
    // all cells must have a local entry (it does not need to be valid)
    EXPECT_TRUE(mapVal.second.hasLocal());
    ++all;
  }
  // all - valid = 1
  EXPECT_EQ(valid + 1, all);
}

TEST_F(DcDMapIteraotrTest, iter_allValid_1) {
  // add new external cell and invalidate it
  update(mapFull, 6, 6, 808, 9, 39.0);
  mapFull.applyVisitorTo(GridCellID(6, 6), ResetVisitor(33.0));
  // invalidate one cell with only local data
  mapFull.applyVisitorTo(GridCellID(1, 1), ResetVisitor(33.0));

  int valid = 0;
  for (auto it = mapFull.valid(); it != it.end(); ++it) {
    auto mapVal = *it;
    EXPECT_TRUE(mapVal.second.hasValid());
    ++valid;
  }

  int all = mapFull.getCells().size();

  // expect 5 cells total
  // expect 3 valid Cells
  EXPECT_EQ(3, valid);
  EXPECT_EQ(5, all);
}

TEST_F(DcDMapIteraotrTest, iter_allValid_2) {
  // GridCellID(6, 6) has two entries. One invalid one valid --> Cell must count
  // as valid. add new external cell and invalidate it
  update(mapFull, 6, 6, 908, 9, 39.0);  // will be invalid
  mapFull.applyVisitorTo(GridCellID(6, 6), ResetVisitor(33.0));
  update(mapFull, 6, 6, 909, 9, 39.0);  // is valid
  // invalidate one cell with only local data
  mapFull.applyVisitorTo(GridCellID(1, 1), ResetVisitor(33.0));

  int valid = 0;
  for (auto it = mapFull.valid(); it != it.end(); ++it) {
    auto mapVal = *it;
    EXPECT_TRUE(mapVal.second.hasValid());
    ++valid;
  }

  int all = mapFull.getCells().size();

  // expect 5 cells total
  // expect 4 valid Cells (see Cell(6, 6))
  EXPECT_EQ(4, valid);
  EXPECT_EQ(5, all);
}

// visitCells tested with other test applyVisitorTo
