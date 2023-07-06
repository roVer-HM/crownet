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

#include "crownet/crownet.h"
#include "crownet/common/Entry.h"
#include "main_test.h"

namespace omnetpp{
    bool operator<(const simtime_t& a, const double b){return a.dbl() < b; }
    bool operator<(const double a, const simtime_t& b){return a < b.dbl();}
}

double TIME1 = 1.34;
double TIME2 = 4.42;

class IEntryTest : public BaseOppTest {
 protected:
  void SetUp() override {
    e1_int_empty = IEntry<int, int>();
    e2_dbl_with_count = IEntry<int, double>(3, TIME1, TIME2);

    e3_int_empty_loc = IGlobalEntry<int, int>();
    e4_dbl_with_count_loc = IGlobalEntry<int, double>(3, TIME1, TIME2);
    e4_dbl_with_count_loc.nodeIds.insert(11);
    e4_dbl_with_count_loc.nodeIds.insert(22);
    e4_dbl_with_count_loc.nodeIds.insert(33);

    e2_dbl_with_count.setSelectedIn("someViewName");
  }

  IEntry<int, int> e1_int_empty;
  IEntry<int, double> e2_dbl_with_count;
  IGlobalEntry<int, int> e3_int_empty_loc;
  IGlobalEntry<int, double> e4_dbl_with_count_loc;

  int t1{1};
  int t2{2};
  int t3{3};
};

TEST_F(IEntryTest, Ctor1) {
  ASSERT_EQ(0, e1_int_empty.getCount());
  ASSERT_EQ(0, e1_int_empty.getMeasureTime());
  ASSERT_EQ(0, e1_int_empty.getReceivedTime());
}

TEST_F(IEntryTest, Ctor2) {
  ASSERT_EQ(3, e2_dbl_with_count.getCount());
  ASSERT_EQ(TIME1, e2_dbl_with_count.getMeasureTime());
  ASSERT_EQ(TIME2, e2_dbl_with_count.getReceivedTime());
}

TEST_F(IEntryTest, isValid) {
  ASSERT_TRUE(e1_int_empty.valid());
  ASSERT_TRUE(e2_dbl_with_count.valid());
}

TEST_F(IEntryTest, reset2) {
  ASSERT_EQ(0, e1_int_empty.getMeasureTime());
  ASSERT_EQ(0, e1_int_empty.getReceivedTime());
  e1_int_empty.reset(t1);
  ASSERT_EQ(0, e1_int_empty.getCount());
  ASSERT_EQ(1, e1_int_empty.getMeasureTime());
  ASSERT_EQ(1, e1_int_empty.getReceivedTime());
  ASSERT_FALSE(e1_int_empty.valid());

  ASSERT_EQ(TIME1, e2_dbl_with_count.getMeasureTime());
  ASSERT_EQ(TIME2, e2_dbl_with_count.getReceivedTime());
  e2_dbl_with_count.reset(t2);
  ASSERT_EQ(0, e2_dbl_with_count.getCount());
  ASSERT_EQ(2, e2_dbl_with_count.getMeasureTime());
  ASSERT_EQ(2, e2_dbl_with_count.getReceivedTime());
  ASSERT_FALSE(e2_dbl_with_count.valid());
}

/**
 * incrementing count will increase count and validate cell
 */
TEST_F(IEntryTest, incrementCount) {
  ASSERT_TRUE(e1_int_empty.valid());  // no count given on construction
  e1_int_empty.incrementCount(2);
  EXPECT_EQ(1, e1_int_empty.getCount());
  EXPECT_EQ(2, e1_int_empty.getMeasureTime());
  EXPECT_EQ(2, e1_int_empty.getReceivedTime());
  EXPECT_TRUE(e1_int_empty.valid());

  EXPECT_TRUE(e2_dbl_with_count.valid());  // has already valid count
  e2_dbl_with_count.incrementCount(2);
  EXPECT_EQ(4, e2_dbl_with_count.getCount());
  EXPECT_EQ(2, e2_dbl_with_count.getMeasureTime());
  EXPECT_EQ(2, e2_dbl_with_count.getReceivedTime());
  EXPECT_TRUE(e2_dbl_with_count.valid());
}

/*
 * A decrement must never invalidate the count
 */
TEST_F(IEntryTest, decrementCount) {
  e1_int_empty.incrementCount(2);
  ASSERT_EQ(1, e1_int_empty.getCount());
  ASSERT_EQ(2, e1_int_empty.getMeasureTime());
  ASSERT_EQ(2, e1_int_empty.getReceivedTime());
  ASSERT_TRUE(e1_int_empty.valid());

  // count = 0 is valid
  e1_int_empty.decrementCount(3);
  ASSERT_EQ(0, e1_int_empty.getCount());
  ASSERT_EQ(3, e1_int_empty.getMeasureTime());
  ASSERT_EQ(3, e1_int_empty.getReceivedTime());
  ASSERT_TRUE(e1_int_empty.valid());
}

TEST_F(IEntryTest, decrementCountErr) {
  e1_int_empty.incrementCount(2);  // count=1
  e1_int_empty.decrementCount(3);  // count=0 (ok)
  try {
    e1_int_empty.decrementCount(4);  // count=-1 (err)
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
  ASSERT_LT(e1_int_empty.compareMeasureTime(other1), 0);
  ASSERT_EQ(e1_int_empty.compareMeasureTime(other2), 0);

  IEntry<int, double> other3{1, TIME1 - 0.4, 1.0};
  IEntry<int, double> other4{1, TIME1, 1};
  ASSERT_GT(e2_dbl_with_count.compareMeasureTime(other3), 0);
  ASSERT_EQ(e2_dbl_with_count.compareMeasureTime(other4), 0);
}


TEST_F(IEntryTest, toCsv) {
  const char* expectVal = "3,1.34,4.42,0,someViewName";
  ASSERT_STREQ(expectVal, e2_dbl_with_count.csv(",").c_str());
}

TEST_F(IEntryTest, clear) {
  EXPECT_TRUE(e2_dbl_with_count.valid());
  EXPECT_GT(e2_dbl_with_count.getCount(), 0);

  e2_dbl_with_count.clear(t1);
  EXPECT_TRUE(e2_dbl_with_count.valid());
  EXPECT_EQ(e2_dbl_with_count.getCount(), 0);

  EXPECT_TRUE(e1_int_empty.valid());
  e1_int_empty.clear(t1);
  EXPECT_TRUE(e1_int_empty.valid());
}

TEST_F(IEntryTest, clearLocal) {
  EXPECT_TRUE(e4_dbl_with_count_loc.valid());
  EXPECT_GT(e4_dbl_with_count_loc.getCount(), 0);
  EXPECT_GT(e4_dbl_with_count_loc.nodeIds.size(), 0);

  e4_dbl_with_count_loc.clear(t1);
  EXPECT_TRUE(e4_dbl_with_count_loc.valid());
  EXPECT_EQ(e4_dbl_with_count_loc.getCount(), 0);
  EXPECT_EQ(e4_dbl_with_count_loc.nodeIds.size(), 0);

  EXPECT_TRUE(e3_int_empty_loc.valid());
  e3_int_empty_loc.clear(t1);
  EXPECT_TRUE(e3_int_empty_loc.valid());
}

TEST_F(IEntryTest, reset) {
  EXPECT_TRUE(e2_dbl_with_count.valid());
  EXPECT_GT(e2_dbl_with_count.getCount(), 0);

  e2_dbl_with_count.reset(t1);
  EXPECT_FALSE(e2_dbl_with_count.valid());
  EXPECT_EQ(e2_dbl_with_count.getCount(), 0);

  EXPECT_TRUE(e1_int_empty.valid());
  e1_int_empty.reset(t1);
  EXPECT_FALSE(e1_int_empty.valid());
}

TEST_F(IEntryTest, resetLocal) {
  EXPECT_TRUE(e4_dbl_with_count_loc.valid());
  EXPECT_GT(e4_dbl_with_count_loc.getCount(), 0);
  EXPECT_GT(e4_dbl_with_count_loc.nodeIds.size(), 0);

  e4_dbl_with_count_loc.reset(t1);
  EXPECT_FALSE(e4_dbl_with_count_loc.valid());
  EXPECT_EQ(e4_dbl_with_count_loc.getCount(), 0);
  EXPECT_EQ(e4_dbl_with_count_loc.nodeIds.size(), 0);

  EXPECT_TRUE(e3_int_empty_loc.valid());
  e3_int_empty_loc.reset(t1);
  EXPECT_FALSE(e3_int_empty_loc.valid());
}

TEST_F(IEntryTest, set_get_RSD){
    IEntry<int, simtime_t> e;
    e.setResourceSharingDomainId(3);
    EXPECT_EQ(3, e.getResourceSharingDomainId());
}

TEST_F(IEntryTest, set_get_RSD_with_pair){
    IEntry<int, simtime_t> e;
    crownet::RsdIdPair rsdPair {{10, 5.0}, {5, 3.0}};

    EXPECT_EQ(-1, e.getResourceSharingDomainId()) << "not set. Must be -1";

    // current time after RSD change at 5.0s
    setSimTime(5.5);
    e.setMeasureTime(5.3); // Measurement done after RSD change at 5.0
    e.setReceivedTime(simTime());
    e.setResourceSharingDomainId(rsdPair);

    EXPECT_EQ(rsdPair.getId(), e.getResourceSharingDomainId());


    setSimTime(5.5);
    e.setMeasureTime(2.0); // Measurement done before RSD change. use previous RSD
    e.setReceivedTime(simTime());
    e.setResourceSharingDomainId(rsdPair);
    EXPECT_EQ(rsdPair.getPrevId(), e.getResourceSharingDomainId());


    setSimTime(5.5);
    rsdPair.previous.id = -1;
    e.setMeasureTime(2.0); // Measurement done before RSD change but previous id is invalid. use new one.
    e.setReceivedTime(simTime());
    e.setResourceSharingDomainId(rsdPair);
    EXPECT_EQ(rsdPair.getId(), e.getResourceSharingDomainId());

}
