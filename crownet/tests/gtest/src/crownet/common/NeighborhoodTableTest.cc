#include "main_test.h"
#include "crownet/common/NeighborhoodTable.h"

#include "gmock/gmock.h"

using namespace crownet;

class MockNeighborhoodTable : public NeighborhoodTable{

  public:
//    MOCK_METHOD(checkTimeToLive,void());
    // (New) MOCK_METHOD(double, Bar, (std::string s), (override));
//    MOCK_METHOD(void, checkTimeToLive,(),(override));
//    MOCK_METHOD(void, scheduleAt, (simetime_t t, cMessage *msg), (override));
    MOCK_METHOD2(scheduleAt,void(simtime_t t, cMessage* msg));
    MOCK_METHOD0(checkTimeToLive, void());
//    virtual void scheduleAt(simtime_t t, cMessage *msg);

    //    old MOCK_METHOD EXAMPLES
    //    MOCK_METHOD0(PenUp, void());
    //    MOCK_METHOD0(PenDown, void());
    //    MOCK_METHOD1(Forward, void(int distance));
    //    MOCK_METHOD1(Turn, void(int degrees));
    //    MOCK_METHOD2(GoTo, void(int x, int y));
    //    MOCK_CONST_METHOD0(GetX, int());
    //    MOCK_CONST_METHOD0(GetY, int());
};

TEST(NeighborhoodTableBase, Test_constructor) {
  NeighborhoodTable nTable;
  double t = nTable.getMaxAge().dbl();
  EXPECT_EQ(t, 0);
}


class NeighborhoodTableTest : public BaseOppTest {
 public:
    NeighborhoodTableTest() {}
};

TEST_F(NeighborhoodTableTest, checkTimeToLive) {
  simtime_t now = simTime().dbl();
  double maxAge = 3.0;//class MockNeighborhoodTable : public NeighborhoodTable{
  //
  //  public:
  //    MOCK_METHOD0(checkTimeToLive,void());
  //};
  NeighborhoodTable nTable;
  nTable.setMaxAge(maxAge);

  NeighborhoodTableEntry e0{0, 1.0, now - maxAge + 2, inet::Coord(0.0,0.0), inet::Coord(0.0,0.0)};
  NeighborhoodTableEntry e1{1, 1.0, now - maxAge + 1, inet::Coord(1.0,0.0), inet::Coord(0.0,0.0)};
  NeighborhoodTableEntry e2{2, 1.0, now - maxAge + 0, inet::Coord(0.0,1.0), inet::Coord(0.0,0.0)};
  NeighborhoodTableEntry e3{3, 1.0, now - maxAge - 1, inet::Coord(1.0,1.0), inet::Coord(0.0,0.0)};
  NeighborhoodTableEntry e4{4, 1.0, now - maxAge - 2, inet::Coord(2.0,0.0), inet::Coord(0.0,0.0)};
  std::map<int, NeighborhoodTableEntry> internalTable = {{0,e0}, {1,e1}, {2,e2}, {3,e3}, {4,e4}};
  nTable.setTable(internalTable);

  nTable.checkTimeToLive();
  EXPECT_TRUE(internalTable.size() > nTable.getTable().size());

  // check table entries
  EXPECT_EQ(nTable.getTable().count(0), 1);
  EXPECT_EQ(nTable.getTable().count(1), 1);
  EXPECT_EQ(nTable.getTable().count(2), 1);
  EXPECT_EQ(nTable.getTable().count(3), 0);
  EXPECT_EQ(nTable.getTable().count(4), 0);
}

TEST_F(NeighborhoodTableTest, handleBeacon) {
  NeighborhoodTable nTable;

  NeighborhoodTableEntry b0{0, 1.0, 2.0, inet::Coord(0.0,0.0), inet::Coord(0.0,0.0)};
  nTable.handleBeacon(std::move(b0));
  EXPECT_EQ(nTable.getTable().size(), 1);
  EXPECT_EQ(nTable.getTable().find(0)->second.getTimeSend().dbl(), b0.getTimeSend().dbl());

  NeighborhoodTableEntry b1{1, 1.0, 3.0, inet::Coord(0.0,1.0), inet::Coord(0.0,0.0)};
  nTable.handleBeacon(std::move(b1));
  EXPECT_EQ(nTable.getTable().size(), 2);
  EXPECT_EQ(nTable.getTable().find(0)->second.getTimeSend().dbl(), b0.getTimeSend().dbl());
  EXPECT_EQ(nTable.getTable().find(1)->second.getTimeSend().dbl(), b1.getTimeSend().dbl());

  NeighborhoodTableEntry b2{0, 2.0, 4.0, inet::Coord(2.0,0.0), inet::Coord(0.0,0.0)};
  nTable.handleBeacon(std::move(b2));
  EXPECT_EQ(nTable.getTable().size(), 2);
  EXPECT_EQ(nTable.getTable().find(0)->second.getTimeSend().dbl(), b2.getTimeSend().dbl());
  EXPECT_EQ(nTable.getTable().find(1)->second.getTimeSend().dbl(), b1.getTimeSend().dbl());

  NeighborhoodTableEntry b3{0, 1.0, 5.0, inet::Coord(2.0,0.0), inet::Coord(0.0,0.0)};
  nTable.handleBeacon(std::move(b3));
  EXPECT_EQ(nTable.getTable().size(), 2);
  EXPECT_EQ(nTable.getTable().find(0)->second.getTimeSend().dbl(), b2.getTimeSend().dbl());
  EXPECT_EQ(nTable.getTable().find(1)->second.getTimeSend().dbl(), b1.getTimeSend().dbl());
  EXPECT_TRUE(1==1);
}

TEST_F(NeighborhoodTableTest, handleMessage) {
  MockNeighborhoodTable mock;
  cMessage* ttl_msg = new cMessage("TitleMessage");
  cMessage* invalid_msg = new cMessage("InvalidMessage");
  simtime_t time = simTime().dbl();
  double maxAge = 1.0;
  // important to define mock spies before invoking the function to test
  EXPECT_CALL(mock, checkTimeToLive()).Times(1);
  EXPECT_CALL(mock, scheduleAt(time + maxAge, ttl_msg)).Times(1);
  mock.setTitleMessage(ttl_msg);
  mock.setMaxAge(maxAge);
  mock.handleMessage(ttl_msg);
  // this tests _that_ the expected exception is thrown
  EXPECT_THROW(mock.handleMessage(invalid_msg), cRuntimeError);
  delete invalid_msg; // suppress weird warning
}

//TEST_F(NeighborhoodTableTest, initialize) {
//  MockNeighborhoodTable mock;
//  cMessage* ttl_msg = new cMessage("NeighborhoodTable_ttl");
//  simtime_t time = simTime().dbl();
//  double maxAge = 1.0;
//  EXPECT_CALL(mock, scheduleAt(time + maxAge, ttl_msg)).Times(1); // triggers on call with arg 0
//  mock.setTitleMessage(ttl_msg);
//  mock.setMaxAge(maxAge);
//
//  mock.initialize(0); // failes on par("maxAge") -> function of cComponent
//  mock.initialize(1);
//  mock.initialize(2);
//  mock.initialize(42);
//}
