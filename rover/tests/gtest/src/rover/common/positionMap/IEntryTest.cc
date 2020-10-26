/*
 * grid_test.cc
 *
 *  Created on: Sep 11, 2020
 *      Author: sts
 */

#include <gtest/gtest.h>
#include <omnetpp.h>
#include <memory>
#include <string>

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

/**
 * incrementing count will increase count and validate cell
 */
TEST_F(IEntryTest, incrementCount) {
  EXPECT_FALSE(e1.valid());  // no count given on construction
  e1.incrementCount(2);
  EXPECT_EQ(1, e1.getCount());
  EXPECT_EQ(2, e1.getMeasureTime());
  EXPECT_EQ(2, e1.getReceivedTime());
  EXPECT_TRUE(e1.valid());

  EXPECT_TRUE(e2.valid());  // has already valid count
  e2.incrementCount(2);
  EXPECT_EQ(4, e2.getCount());
  EXPECT_EQ(2, e2.getMeasureTime());
  EXPECT_EQ(2, e2.getReceivedTime());
  EXPECT_TRUE(e2.valid());
}

/*
 * A decrement must never invalidate the count
 */
TEST_F(IEntryTest, decrementCount) {
  e1.incrementCount(2);
  ASSERT_EQ(1, e1.getCount());
  ASSERT_EQ(2, e1.getMeasureTime());
  ASSERT_EQ(2, e1.getReceivedTime());
  ASSERT_TRUE(e1.valid());

  // count = 0 is valid
  e1.decrementCount(3);
  ASSERT_EQ(0, e1.getCount());
  ASSERT_EQ(3, e1.getMeasureTime());
  ASSERT_EQ(3, e1.getReceivedTime());
  ASSERT_TRUE(e1.valid());
}

TEST_F(IEntryTest, decrementCountErr) {
  e1.incrementCount(2);  // count=1
  e1.decrementCount(3);  // count=0 (ok)
  try {
    e1.decrementCount(4);  // count=-1 (err)
    FAIL() << "negative count must be an error\n";
  } catch (omnetpp::cRuntimeError& e) {
  } catch (...) {
    FAIL() << " Unexpected exception thrown " << std::current_exception
           << std::endl;
  }
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

TEST_F(IEntryTest, XXXX) {
  //   std::shared_ptr<LocalDensityMeasure<NODE_ID>> locMeasure =
  //        std::static_pointer_cast<LocalDensityMeasure<NODE_ID>>(
  //            this->getCellEntry(cellId).getLocal());

  using OppEntry = IEntry<std::string, omnetpp::simtime_t>;
  using OppLocEntry = ILocalEntry<std::string, omnetpp::simtime_t>;

  std::shared_ptr<OppEntry> e1 = std::make_shared<OppEntry>(1, 1.1, 2.2);
  std::shared_ptr<OppEntry> e2 = std::make_shared<OppLocEntry>(1, 1.1, 2.2);

  IEntry<std::string, omnetpp::simtime_t> other1{1, 1.1, 2.2};
  std::cout << "start";
  other1.reset();
  e1->reset();
  std::cout << "end" << std::endl;
  e2->reset();  // print
  std::shared_ptr<OppLocEntry> e2Cast =
      std::static_pointer_cast<OppLocEntry>(e2);
  e2Cast->reset();  // print
}

TEST_F(IEntryTest, toCsv) {
  const char* expectVal = "3,1.34,4.42,0";
  ASSERT_STREQ(expectVal, e2.csv(",").c_str());
}
