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
#include "crownet/dcd/regularGrid/RegularCell.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/common/RegularGridInfo.h"

using namespace crownet;

namespace {

RegularGridInfo  grid{{10.0, 10.0}, {1.0, 1.0}};
RegularDcdMapFactory dcdFactory{grid};

}


TEST(GenricCell, Test_constructor) {
  Cell<int, int, SimTime> cell{dcdFactory.getTimeProvider(), 12, 13};
  EXPECT_EQ(cell.getCellId(), 12);
  EXPECT_EQ(cell.getOwnerId(), 13);
}

TEST(GenricCell, Test_str) {
  Cell<int, int, SimTime> cell{dcdFactory.getTimeProvider(), 12, 13};
  std::string s = "{cell_id: 12 owner_id: 13}";
  EXPECT_STREQ(cell.str().c_str(), s.c_str());
}

// use RegularCell

class RegularCellTest : public BaseOppTest {
 public:
  RegularCellTest()
      : cell(dcdFactory.getTimeProvider(), GridCellID(5, 4), IntIdentifer(42)),
        cellEmpty(dcdFactory.getTimeProvider(), GridCellID(6, 5), IntIdentifer(42)) {}
  void SetUp() override {
    auto& m = cell.getData();
    // add data (node_key_t (owner), IEntry (count data)
    m[IntIdentifer(14)] = ctor.entry();  // count 2
    m[IntIdentifer(14)]->incrementCount(3);
    m[IntIdentifer(14)]->incrementCount(3);
    m[IntIdentifer(14)]->setSource(14);
    m[IntIdentifer(14)]->setEntryDist(EntryDist{5.0, 3.0, 4.0}); // w=1/3.0

    m[IntIdentifer(15)] = ctor.entry();  // count 1
    m[IntIdentifer(15)]->incrementCount(1);
    m[IntIdentifer(15)]->setSource(15);
    m[IntIdentifer(15)]->setEntryDist(EntryDist{5.0, 10.0, 4.0}); // w=1/10.0


    m[IntIdentifer(16)] = ctor.entry();  // count 2
    m[IntIdentifer(16)]->incrementCount(1);
    m[IntIdentifer(16)]->incrementCount(1);
    m[IntIdentifer(16)]->setSource(16);
    m[IntIdentifer(16)]->setEntryDist(EntryDist{5.0, 2.0, 4.0}); // w=1/2.0


    m[IntIdentifer(17)] = ctor.entry();  // count 1
    m[IntIdentifer(17)]->incrementCount(2);
    m[IntIdentifer(17)]->setSource(17);
    m[IntIdentifer(17)]->setEntryDist(EntryDist{5.0, 20.0, 4.0}); // w=1/20.0



    auto gentry = ctor.globalEntry();
    m[IntIdentifer(42)] = gentry;  // count 4
    gentry->incrementCount(1);
    gentry->nodeIds.insert(5);
    gentry->incrementCount(1);
    gentry->nodeIds.insert(9);
    gentry->incrementCount(1);
    gentry->nodeIds.insert(3);
    gentry->incrementCount(1);
    gentry->nodeIds.insert(19);
    gentry->setSource(42);
    gentry->setEntryDist(EntryDist{}); // {0, 0, 0}  w=0.0 -> 1.0/1.0 (same cell)
  }

 protected:
  RegularCell cell;
  RegularCell cellEmpty;
  RegularCell::entry_ctor_t ctor;
};

TEST_F(RegularCellTest, empytIter) {
  EXPECT_TRUE(cellEmpty.begin() == cellEmpty.end());
  EXPECT_TRUE(cellEmpty.validIter().begin() == cellEmpty.validIter().end());
}

TEST_F(RegularCellTest, getData) { EXPECT_EQ(5, cell.getData().size()); }

TEST_F(RegularCellTest, getData_emtpy) {
  EXPECT_EQ(0, cellEmpty.getData().size());
}

TEST_F(RegularCellTest, hasData_True) { EXPECT_TRUE(cell.hasData(14)); }

TEST_F(RegularCellTest, hasData_False) { EXPECT_FALSE(cell.hasData(500)); }

TEST_F(RegularCellTest, hasLocal_True) { EXPECT_TRUE(cell.hasLocal()); }

TEST_F(RegularCellTest, hasLocal_False) { EXPECT_FALSE(cellEmpty.hasLocal()); }

TEST_F(RegularCellTest, hasValid_True) { EXPECT_TRUE(cell.hasValid()); }
TEST_F(RegularCellTest, hasValid_False) {
  // no entry implies invalid entries
  EXPECT_FALSE(cellEmpty.hasValid());

  // Invalidate all entries in cell
  cell.acceptSet(ResetVisitor(111.1));
  // Existing but invalid data
  EXPECT_FALSE(cell.hasValid());
}

TEST_F(RegularCellTest, hasValidLocal_True) {
  EXPECT_TRUE(cell.hasValidLocal());
}
TEST_F(RegularCellTest, hasValidLocal_False) {
  // no entry implies invalid local entry
  EXPECT_FALSE(cellEmpty.hasValidLocal());

  // Invalidate all entries in cell
  cell.acceptSet(ResetVisitor(111.1));
  // Existing but invalid local data
  EXPECT_FALSE(cell.hasValidLocal());
}

TEST_F(RegularCellTest, getLocal_Fail) {
  try {
    auto e = cellEmpty.getLocal();
    FAIL() << "omnetpp::cRuntimeError expected" << std::endl;
  } catch (omnetpp::cRuntimeError e) {
  } catch (...) {
    FAIL() << "unexpected throw" << std::endl;
  }
}

TEST_F(RegularCellTest, getLocal_Success) {
  auto e = cell.getLocal();
  EXPECT_EQ(IntIdentifer(42), e->getSource());
}

TEST_F(RegularCellTest, get_Fail_onEmpyt) {
  try {
    auto e = cellEmpty.get(222);  // none existing IntIdentifier;
    FAIL() << "omnetpp::cRuntimeError expected" << std::endl;
  } catch (omnetpp::cRuntimeError e) {
  } catch (...) {
    FAIL() << "unexpected throw" << std::endl;
  }
}

TEST_F(RegularCellTest, get_Fail_withData) {
  try {
    auto e = cell.get(222);  // none existing IntIdentifier;
    FAIL() << "omnetpp::cRuntimeError expected" << std::endl;
  } catch (omnetpp::cRuntimeError e) {
  } catch (...) {
    FAIL() << "unexpected throw" << std::endl;
  }
}

TEST_F(RegularCellTest, get_Success) {
  auto e = cell.get(14);
  EXPECT_EQ(IntIdentifer(14), e->getSource());
}

TEST_F(RegularCellTest, get_set_CellId) {
  EXPECT_EQ(GridCellID(5, 4), cell.getCellId());
  cell.setCellId(GridCellID(99, 99));
  EXPECT_EQ(GridCellID(99, 99), cell.getCellId());
}

TEST_F(RegularCellTest, get_set_OwnerId) {
  EXPECT_EQ(IntIdentifer(42), cell.getOwnerId());
  cell.setOwnerId(IntIdentifer(99));
  EXPECT_EQ(IntIdentifer(99), cell.getOwnerId());
}

TEST_F(RegularCellTest, put_new) {
  auto entry = ctor.entry();
  entry->setSource(500);
  cell.put(entry);
  EXPECT_EQ(entry, cell.get(500));
}

TEST_F(RegularCellTest, put_override) {
  auto entry = ctor.entry();
  entry->setSource(14);
  auto entryOld = cell.get(14);

  cell.put(entry);
  EXPECT_EQ(entry, cell.get(14));
  EXPECT_NE(entryOld, cell.get(14));
}
//
TEST_F(RegularCellTest, put_move) {
  auto entry = ctor.entry();
  entry->setSource(500);
  cell.put(std::move(entry));
  EXPECT_EQ(IntIdentifer(500), cell.get(500)->getSource());
  EXPECT_TRUE(entry == nullptr);
}

TEST_F(RegularCellTest, put_override_move) {
  auto entry = ctor.entry();
  entry->setSource(14);
  auto entryOld = cell.get(14);

  cell.put(std::move(entry));
  EXPECT_EQ(IntIdentifer(14), cell.get(14)->getSource());
  EXPECT_NE(entryOld->getCount(), cell.get(14)->getCount());
  EXPECT_TRUE(entry == nullptr);
}

TEST_F(RegularCellTest, increment_onEmptyEntry) {
  cellEmpty.getOrCreate()->incrementCount(10.0); //empty must create
  EXPECT_TRUE(cellEmpty.hasLocal());
  auto lEntry = cellEmpty.getLocal();
  EXPECT_EQ(1, lEntry->getCount());
  EXPECT_DOUBLE_EQ(10.0, lEntry->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(10.0, lEntry->getReceivedTime().dbl());
}

TEST_F(RegularCellTest, increment_withDataInEntry) {
  auto lEntry = cell.get(); // local (must exist)
  EXPECT_EQ(4, lEntry->getCount());
  cell.get()->incrementCount(10.0); // time
  EXPECT_EQ(5, lEntry->getCount());
  EXPECT_DOUBLE_EQ(10.0, lEntry->getMeasureTime().dbl());
  EXPECT_DOUBLE_EQ(10.0, lEntry->getReceivedTime().dbl());
}

class RegularCellComparisonTest : public BaseOppTest {
 public:
  RegularCellComparisonTest()
      : cella_44(dcdFactory.getTimeProvider(), GridCellID(3, 4), IntIdentifer(44)),
        cella_55(dcdFactory.getTimeProvider(), GridCellID(3, 4), IntIdentifer(55)),
        cellb_44(dcdFactory.getTimeProvider(), GridCellID(3, 7), IntIdentifer(44)),
        cellc_44(dcdFactory.getTimeProvider(), GridCellID(4, 0), IntIdentifer(44)),
        celld_44(dcdFactory.getTimeProvider(), GridCellID(9, 9), IntIdentifer(44)) {}
  void SetUp() override {}

 protected:
  // total order: cella_44 < cella_55 < cellb_44 < cellc_44, < celld_44
  RegularCell cella_44;
  RegularCell cella_55;
  RegularCell cellb_44;
  RegularCell cellc_44;
  RegularCell celld_44;
};

TEST_F(RegularCellComparisonTest, equality_1) {
  EXPECT_TRUE(cella_44 == cella_44);
  EXPECT_TRUE(cella_55 == cella_55);
  EXPECT_TRUE(cellb_44 == cellb_44);
  EXPECT_TRUE(cellc_44 == cellc_44);
  EXPECT_TRUE(celld_44 == celld_44);
}

TEST_F(RegularCellComparisonTest, equality_2) {
  EXPECT_FALSE(cella_44 == cella_55);
  EXPECT_FALSE(cella_55 == cellb_44);
  EXPECT_FALSE(cellb_44 == cellc_44);
  EXPECT_FALSE(cellc_44 == celld_44);
  EXPECT_FALSE(celld_44 == cella_44);
}

/**
 * Print all entries
 */
TEST_F(RegularCellTest, printVisitorAll) {
  CellPrinterAll<RegularCell> printAll;
  std::string cell_str =
      cell.accept<CellPrinterAll<RegularCell>, std::string>(printAll);

  std::string ret =
      "{cell_id: [5,4] owner_id: 42}\n"
      "  14 --> Count: 2| meas_t: 3| recv_t: 3| valid: 1\n"
      "  15 --> Count: 1| meas_t: 1| recv_t: 1| valid: 1\n"
      "  16 --> Count: 2| meas_t: 1| recv_t: 1| valid: 1\n"
      "  17 --> Count: 1| meas_t: 2| recv_t: 2| valid: 1\n"
      "  42 --> Count: 4| meas_t: 1| recv_t: 1| valid: 1| node_ids: {3, 5, 9, "
      "19}\n";
  EXPECT_STREQ(cell_str.c_str(), ret.c_str());
}

/**
 * Print only valid data entries
 */
TEST_F(RegularCellTest, printVisitorValid) {
  CellPrinterValid<RegularCell> printValid;
  cell.getData()[IntIdentifer(16)]->reset(5);  // at time=5

  std::string cell_str =
      cell.accept<CellPrinterValid<RegularCell>, std::string>(printValid);

  std::string ret =
      "{cell_id: [5,4] owner_id: 42}(valid only)\n"
      "  14 --> Count: 2| meas_t: 3| recv_t: 3| valid: 1\n"
      "  15 --> Count: 1| meas_t: 1| recv_t: 1| valid: 1\n"
      "  17 --> Count: 1| meas_t: 2| recv_t: 2| valid: 1\n"
      "  42 --> Count: 4| meas_t: 1| recv_t: 1| valid: 1| node_ids: {3, 5, 9, "
      "19}\n";
  EXPECT_STREQ(cell_str.c_str(), ret.c_str());
}

TEST_F(RegularCellTest, clearVisitor) {
  ClearVisitor clearVisitor(15);  // at time 15
  cell.acceptSet(clearVisitor);

  CellPrinterAll<RegularCell> printAll;
  std::string cell_str =
      cell.accept<CellPrinterAll<RegularCell>, std::string>(printAll);

  // expect count 0, updated time and all valid.
  cell.acceptSet([](const RegularCell& cell) {
    for (const auto& e : cell) {
      EXPECT_EQ(e.second->getCount(), 0);
      EXPECT_EQ(e.second->getMeasureTime(), 15);
      EXPECT_EQ(e.second->getReceivedTime(), 15);
      EXPECT_TRUE(e.second->valid());
    }
  });
}

TEST_F(RegularCellTest, resetVisitor) {
  ResetVisitor resetVisitor(33);  // at time 33
  cell.acceptSet(resetVisitor);

  CellPrinterAll<RegularCell> printAll;
  std::string cell_str =
      cell.accept<CellPrinterAll<RegularCell>, std::string>(printAll);

  // expect count 0, updated time and all invalid.
  cell.acceptSet([](const RegularCell& cell) {
    for (const auto& e : cell) {
      EXPECT_EQ(e.second->getCount(), 0);
      EXPECT_EQ(e.second->getMeasureTime(), 33);
      EXPECT_EQ(e.second->getReceivedTime(), 33);
      EXPECT_FALSE(e.second->valid());
    }
  });
}

TEST_F(RegularCellTest, ymfVisitor) {
  YmfVisitor ymf_v;
  auto data_14 = cell.getData()[IntIdentifer(14)];  // ymf value
  auto expect_14 = cell.acceptGetEntry(ymf_v);
  EXPECT_EQ(*(expect_14), *data_14);

  data_14->reset(12);  // if [14] is invalid [17]
  auto expect_17 = cell.acceptGetEntry(ymf_v);
  auto data_17 = cell.getData()[IntIdentifer(17)];
  EXPECT_EQ(*(expect_17), *data_17);
}

TEST_F(RegularCellTest, ymfVisitor_2) {
  std::shared_ptr<YmfVisitor> ymf_v = std::make_shared<YmfVisitor>(simTime());
  auto data_14 = cell.getData()[IntIdentifer(14)];  // ymf value
  cell.computeValue(ymf_v);
  EXPECT_EQ(*(cell.val()), *data_14);

  data_14->reset(12);  // if [14] is invalid [17]
  cell.computeValue(ymf_v);
  auto data_17 = cell.getData()[IntIdentifer(17)];
  EXPECT_EQ(*(cell.val()), *data_17);
}

TEST_F(RegularCellTest, meanVisitor_1) {
    setSimTime(112.0);
    std::shared_ptr<MeanVisitor> visitor = std::make_shared<MeanVisitor>(simTime());
    // calculate mean count of all valid measurements in the given cell.
    // cell mean: (2 + 1 + 2 + 1 + 4)/5
    // time mean: (3 + 1 + 1 + 2 + 1)/5 = 1.6
    cell.computeValue(visitor);
    EXPECT_EQ(cell.val()->getCount(), 10/5);
    EXPECT_DOUBLE_EQ(cell.val()->getMeasureTime().dbl(), (3 + 1 + 1 + 2 + 1)/5.);
    EXPECT_DOUBLE_EQ(cell.val()->getReceivedTime().dbl(), (3 + 1 + 1 + 2 + 1)/5.);

    auto& m = cell.getData();
    m[IntIdentifer(17)]->incrementCount(10);
    visitor->setTime(115.0);
    // increment count and time. -> must change calcualted value
    // cell mean: (2 + 1 + 2 + >2< + 4)/5
    // time mean: (3 + 1 + 1 + >10< + 1)/5
    cell.computeValue(visitor);
    EXPECT_EQ(cell.val()->getCount(), (double)11/5);
    EXPECT_DOUBLE_EQ(cell.val()->getMeasureTime().dbl(), (3 + 1 + 1 + 10 + 1)/5.);
    EXPECT_DOUBLE_EQ(cell.val()->getReceivedTime().dbl(), (3 + 1 + 1 + 10 + 1)/5.);
}

TEST_F(RegularCellTest, meanVisitor_2) {
    setSimTime(112.0);
    std::shared_ptr<MeanVisitor> visitor = std::make_shared<MeanVisitor>(simTime());
    // calculate mean count of all valid measurements in the given cell.
    // cell mean: (2 + 1 + 2 + 1)/4
    // time mean: (3 + 1 + 1 + 2)/4 = 1.75
    auto& m = cell.getData();
    // invalidate entry which thus must not be part of the average
    m[IntIdentifer(42)]->reset();
    cell.computeValue(visitor);
    EXPECT_EQ(cell.val()->getCount(), (double)(2+1+2+1)/4);
    EXPECT_DOUBLE_EQ(cell.val()->getMeasureTime().dbl(), (3 + 1 + 1 + 2)/4.);
    EXPECT_DOUBLE_EQ(cell.val()->getReceivedTime().dbl(), (3 + 1 + 1 + 2)/4.);

}

TEST_F(RegularCellTest, medianVisitor_1) {
    setSimTime(112.0);
    std::shared_ptr<MedianVisitor> visitor = std::make_shared<MedianVisitor>(simTime());
    // calculate mean count of all valid measurements in the given cell.
    // (2 + 1 + 2 + 1 + 4)
    cell.computeValue(visitor);
    EXPECT_EQ(cell.val()->getCount(), 2);
}

TEST_F(RegularCellTest, medianVisitor_2) {
    setSimTime(112.0);
    std::shared_ptr<MedianVisitor> visitor = std::make_shared<MedianVisitor>(simTime());
    // calculate mean count of all valid measurements in the given cell.
    auto& m = cell.getData();
    // invalidate entry which thus must not be part of the average
    m[IntIdentifer(42)]->reset();
    // [1, 1, 2, 2]
    cell.computeValue(visitor);
    EXPECT_EQ(cell.val()->getCount(), 1.5);
}

TEST_F(RegularCellTest, maxVisitor_1) {
    setSimTime(112.0);
    std::shared_ptr<MaxCountVisitor> visitor = std::make_shared<MaxCountVisitor>(simTime());
    // calculate mean count of all valid measurements in the given cell.
    // cell mean: (2 + 1 + 2 + 1 + 4)/5
    cell.computeValue(visitor);
    EXPECT_EQ(cell.val()->getCount(), 4);
}

// 2.0/3.0 + 1.0*/10.0 + 2.0/2.0 + 1.0/20.0


TEST_F(RegularCellTest, invSourceDistVisitor_1) {
    setSimTime(112.0);
    std::shared_ptr<InvSourceDistVisitor> visitor = std::make_shared<InvSourceDistVisitor>(simTime());
    double wSum = 1.0/3.0 + 1.0/10.0 + 1.0/2.0 + 1.0/20.0 + 1.0;
    double entrySum = 2.0/3.0 + 1.0/10.0 + 2.0/2.0 + 1.0/20.0 + 4.0/1.0;
    double wMean =  entrySum/wSum;
    cell.computeValue(visitor);
    EXPECT_EQ(cell.val()->getCount(), wMean);
}
