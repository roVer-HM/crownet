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

class RegularCellVisitorTest : public CellTest {
 public:
  RegularCellVisitorTest(): CellTest() {}

  void SetUp() override {
      init_grid();
      setup_cells();
  }

};

/**
 * Print all entries
 */
TEST_F(RegularCellVisitorTest, printVisitorAll) {
  CellPrinterAll<RegularCell> printAll;
  std::string cell_str =
      cell.accept<CellPrinterAll<RegularCell>, std::string>(printAll);

  std::string ret =
      "{cell_id: [5, 4] owner_id: 42, val:n/a}\n"
      "  14 --> [Count: 2, meas_t: 3, recv_t: 3, valid: 1, rsd: 1]\n"
      "  15 --> [Count: 1, meas_t: 1, recv_t: 1, valid: 1, rsd: 1]\n"
      "  16 --> [Count: 2, meas_t: 1, recv_t: 1, valid: 1, rsd: 1]\n"
      "  17 --> [Count: 1, meas_t: 2, recv_t: 2, valid: 1, rsd: 1]\n"
      "  42 --> [Count: 4, meas_t: 1, recv_t: 1, valid: 1, rsd: 1] node_ids: {3, 5, 9, 19}\n\n";
  EXPECT_STREQ(cell_str.c_str(), ret.c_str());
}

/**
 * Print only valid data entries
 */
TEST_F(RegularCellVisitorTest, printVisitorValid) {
  CellPrinterValid<RegularCell> printValid;
  cell.getData()[IntIdentifer(16)]->reset(5);  // at time=5

  std::string cell_str =
      cell.accept<CellPrinterValid<RegularCell>, std::string>(printValid);

  std::string ret =
      "{cell_id: [5, 4] owner_id: 42, val:n/a}(valid only)\n"
      "  14 --> [Count: 2, meas_t: 3, recv_t: 3, valid: 1, rsd: 1]\n"
      "  15 --> [Count: 1, meas_t: 1, recv_t: 1, valid: 1, rsd: 1]\n"
      "  17 --> [Count: 1, meas_t: 2, recv_t: 2, valid: 1, rsd: 1]\n"
      "  42 --> [Count: 4, meas_t: 1, recv_t: 1, valid: 1, rsd: 1] node_ids: {3, 5, 9, 19}\n";
  EXPECT_STREQ(cell_str.c_str(), ret.c_str());
}

TEST_F(RegularCellVisitorTest, clearVisitor) {
  ClearVisitor clearVisitor(15);  // at time 15
//  cell.acceptSet(clearVisitor);
  cell.acceptSet(ClearVisitor(15));

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

TEST_F(RegularCellVisitorTest, resetVisitor) {
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

TEST_F(RegularCellVisitorTest, ymfVisitor) {
  YmfVisitor ymf_v;
  auto data_14 = cell.getData()[IntIdentifer(14)];  // ymf value
  auto expect_14 = cell.acceptGetEntry(ymf_v);
  EXPECT_EQ(*(expect_14), *data_14);

  data_14->reset(12);  // if [14] is invalid [17]
  auto expect_17 = cell.acceptGetEntry(ymf_v);
  auto data_17 = cell.getData()[IntIdentifer(17)];
  EXPECT_EQ(*(expect_17), *data_17);
}

TEST_F(RegularCellVisitorTest, ymfVisitor_2) {
  std::shared_ptr<YmfVisitor> ymf_v = std::make_shared<YmfVisitor>(simTime());
  auto data_14 = cell.getData()[IntIdentifer(14)];  // ymf value
  cell.computeValue(ymf_v);
  EXPECT_EQ(*(cell.val()), *data_14);

  data_14->reset(12);  // if [14] is invalid [17]
  cell.computeValue(ymf_v);
  auto data_17 = cell.getData()[IntIdentifer(17)];
  EXPECT_EQ(*(cell.val()), *data_17);
}

TEST_F(RegularCellVisitorTest, ymfPlustDist_1) {
    auto m = cell.getData();
    m[IntIdentifer(14)]->setTime(1.0);
    m[IntIdentifer(15)]->setTime(2.0);
    m[IntIdentifer(16)]->setTime(3.0);
    m[IntIdentifer(17)]->setTime(4.0);
    m[IntIdentifer(42)]->reset();
    double sum = 4. + 3. + 2. + 1. - (4*1.);
    setSimTime(5.0);
    std::shared_ptr<YmfPlusDistVisitor> ymfP_v = std::make_shared<YmfPlusDistVisitor>();
    ymfP_v->setTime(5.0);
    auto ret = ymfP_v->getSums(cell);
    EXPECT_EQ(ret.age_sum, sum);

}

TEST_F(RegularCellVisitorTest, meanVisitor_1) {
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

TEST_F(RegularCellVisitorTest, meanVisitor_2) {
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

TEST_F(RegularCellVisitorTest, medianVisitor_1) {
    setSimTime(112.0);
    std::shared_ptr<MedianVisitor> visitor = std::make_shared<MedianVisitor>(simTime());
    // calculate mean count of all valid measurements in the given cell.
    // (2 + 1 + 2 + 1 + 4)
    cell.computeValue(visitor);
    EXPECT_EQ(cell.val()->getCount(), 2);
}

TEST_F(RegularCellVisitorTest, medianVisitor_2) {
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

TEST_F(RegularCellVisitorTest, maxVisitor_1) {
    setSimTime(112.0);
    std::shared_ptr<MaxCountVisitor> visitor = std::make_shared<MaxCountVisitor>(simTime());
    // calculate mean count of all valid measurements in the given cell.
    // cell mean: (2 + 1 + 2 + 1 + 4)/5
    cell.computeValue(visitor);
    EXPECT_EQ(cell.val()->getCount(), 4);
}

// 2.0/3.0 + 1.0*/10.0 + 2.0/2.0 + 1.0/20.0


TEST_F(RegularCellVisitorTest, invSourceDistVisitor_1) {
    setSimTime(112.0);
    std::shared_ptr<InvSourceDistVisitor> visitor = std::make_shared<InvSourceDistVisitor>(simTime());
    double wSum = 1.0/3.0 + 1.0/10.0 + 1.0/2.0 + 1.0/20.0 + 1.0;
    double entrySum = 2.0/3.0 + 1.0/10.0 + 2.0/2.0 + 1.0/20.0 + 4.0/1.0;
    double wMean =  entrySum/wSum;
    cell.computeValue(visitor);
    EXPECT_EQ(cell.val()->getCount(), wMean);
}
