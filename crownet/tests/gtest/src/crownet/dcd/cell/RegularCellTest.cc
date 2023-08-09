/*
 * CellEntryTest.cc
 *
 *  Created on: Oct 8, 2020
 *      Author: sts
 */

#include <functional>
#include <memory>
#include <string>

#include "main_test.h"
#include "crownet/crownet_testutil.h"
#include "crownet/dcd/regularGrid/RegularCell.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/dcd/regularGrid/MapCellAggregationAlgorithms.h"
#include "crownet/common/RegularGridInfo.h"

using namespace crownet;


// use RegularCell

class RegularCellTest2 : public CellTest {
 public:
  RegularCellTest2(): CellTest() {}

  void SetUp() override {
      init_grid();
      setup_cells();
  }

};

TEST_F(RegularCellTest2, empytIter) {
  EXPECT_TRUE(cellEmpty.begin() == cellEmpty.end());
  EXPECT_TRUE(cellEmpty.validIter().begin() == cellEmpty.validIter().end());
}

TEST_F(RegularCellTest2, getData) { EXPECT_EQ(5, cell.getData().size()); }

TEST_F(RegularCellTest2, getData_emtpy) {
  EXPECT_EQ(0, cellEmpty.getData().size());
}

TEST_F(RegularCellTest2, hasData_True) { EXPECT_TRUE(cell.hasData(14)); }

TEST_F(RegularCellTest2, hasData_False) { EXPECT_FALSE(cell.hasData(500)); }

TEST_F(RegularCellTest2, hasLocal_True) { EXPECT_TRUE(cell.hasLocal()); }

TEST_F(RegularCellTest2, hasLocal_False) { EXPECT_FALSE(cellEmpty.hasLocal()); }

TEST_F(RegularCellTest2, hasLocal_False2) { EXPECT_FALSE(cellNoLocal.hasLocal()); }


TEST_F(RegularCellTest2, hasValid_True) { EXPECT_TRUE(cell.hasValid()); }
TEST_F(RegularCellTest2, hasValid_False) {
  // no entry implies invalid entries
  EXPECT_FALSE(cellEmpty.hasValid());

  // Invalidate all entries in cell
  ResetVisitor rv(111.1);
  cell.acceptSet(rv);
  // Existing but invalid data
  EXPECT_FALSE(cell.hasValid());
}

TEST_F(RegularCellTest2, hasValidLocal_True) {
  EXPECT_TRUE(cell.hasValidLocal());
}
TEST_F(RegularCellTest2, hasValidLocal_False) {
  // no entry implies invalid local entry
  EXPECT_FALSE(cellEmpty.hasValidLocal());

  // Invalidate all entries in cell
  ResetVisitor rv(111.1);
  cell.acceptSet(rv);
  // Existing but invalid local data
  EXPECT_FALSE(cell.hasValidLocal());
}

TEST_F(RegularCellTest2, getLocal_Fail) {
  try {
    auto e = cellEmpty.getLocal();
    FAIL() << "omnetpp::cRuntimeError expected" << std::endl;
  } catch (omnetpp::cRuntimeError e) {
  } catch (...) {
    FAIL() << "unexpected throw" << std::endl;
  }
}

TEST_F(RegularCellTest2, getLocal_Success) {
  auto e = cell.getLocal();
  EXPECT_EQ(IntIdentifer(42), e->getSource());
}

TEST_F(RegularCellTest2, get_Fail_onEmpyt) {
  try {
    auto e = cellEmpty.get(222);  // none existing IntIdentifier;
    FAIL() << "omnetpp::cRuntimeError expected" << std::endl;
  } catch (omnetpp::cRuntimeError e) {
  } catch (...) {
    FAIL() << "unexpected throw" << std::endl;
  }
}

TEST_F(RegularCellTest2, get_Fail_withData) {
  try {
    auto e = cell.get(222);  // none existing IntIdentifier;
    FAIL() << "omnetpp::cRuntimeError expected" << std::endl;
  } catch (omnetpp::cRuntimeError e) {
  } catch (...) {
    FAIL() << "unexpected throw" << std::endl;
  }
}

TEST_F(RegularCellTest2, get_Success) {
  auto e = cell.get(14);
  EXPECT_EQ(IntIdentifer(14), e->getSource());
}

TEST_F(RegularCellTest2, get_set_CellId) {
  EXPECT_EQ(GridCellID(5, 4), cell.getCellId());
  cell.setCellId(GridCellID(99, 99));
  EXPECT_EQ(GridCellID(99, 99), cell.getCellId());
}

TEST_F(RegularCellTest2, get_set_OwnerId) {
  EXPECT_EQ(IntIdentifer(42), cell.getOwnerId());
  cell.setOwnerId(IntIdentifer(99));
  EXPECT_EQ(IntIdentifer(99), cell.getOwnerId());
}

TEST_F(RegularCellTest2, put_new) {
  auto entry = ctor.entry();
  entry->setSource(500);
  cell.put(entry);
  EXPECT_EQ(entry, cell.get(500));
}

TEST_F(RegularCellTest2, put_override) {
  auto entry = ctor.entry();
  entry->setSource(14);
  auto entryOld = cell.get(14);

  cell.put(entry);
  EXPECT_EQ(entry, cell.get(14));
  EXPECT_NE(entryOld, cell.get(14));
}
//
TEST_F(RegularCellTest2, put_move) {
  auto entry = ctor.entry();
  entry->setSource(500);
  cell.put(std::move(entry));
  EXPECT_EQ(IntIdentifer(500), cell.get(500)->getSource());
  EXPECT_TRUE(entry == nullptr);
}

TEST_F(RegularCellTest2, put_override_move) {
  auto entry = ctor.entry();
  entry->setSource(14);
  auto entryOld = cell.get(14);

  cell.put(std::move(entry));
  EXPECT_EQ(IntIdentifer(14), cell.get(14)->getSource());
  EXPECT_NE(entryOld->getCount(), cell.get(14)->getCount());
  EXPECT_TRUE(entry == nullptr);
}

TEST_F(RegularCellTest2, increment_onEmptyEntry) {
  cellEmpty.getOrCreate()->incrementCount(10.0); //empty must create
  EXPECT_TRUE(cellEmpty.hasLocal());
  auto lEntry = cellEmpty.getLocal();
  EXPECT_EQ(1, lEntry->getCount());
  EXPECT_DOUBLE_EQ(10.0, lEntry->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(10.0, lEntry->getReceivedTime().dbl());
}

TEST_F(RegularCellTest2, increment_withDataInEntry) {
  auto lEntry = cell.get(); // local (must exist)
  EXPECT_EQ(4, lEntry->getCount());
  cell.get()->incrementCount(10.0); // time
  EXPECT_EQ(5, lEntry->getCount());
  EXPECT_DOUBLE_EQ(10.0, lEntry->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(10.0, lEntry->getReceivedTime().dbl());
}


