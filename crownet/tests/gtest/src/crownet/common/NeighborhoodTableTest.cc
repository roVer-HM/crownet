#include "main_test.h"
#include "crownet/common/NeighborhoodTable.h"

#include "gmock/gmock.h"

using namespace crownet;
using ::testing::Return;

class MockNeighborhoodTable : public NeighborhoodTable{

  public:
    MOCK_METHOD2(scheduleAt, void(simtime_t t, cMessage* msg));
    MOCK_METHOD0(checkTimeToLive, void());
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
  double maxAge = 3.0;
  NeighborhoodTable nTable;
  nTable.setMaxAge(maxAge);

  NeighborhoodTableEntry e0{0, now - maxAge + 2, now - maxAge + 2, inet::Coord(0.0,0.0), inet::Coord(0.0,0.0)};
  NeighborhoodTableEntry e1{1, now - maxAge + 1, now - maxAge + 1, inet::Coord(1.0,0.0), inet::Coord(0.0,0.0)};
  NeighborhoodTableEntry e2{2, now - maxAge + 0, now - maxAge + 0, inet::Coord(0.0,1.0), inet::Coord(0.0,0.0)};
  NeighborhoodTableEntry e3{3, now - maxAge - 1, now - maxAge - 1, inet::Coord(1.0,1.0), inet::Coord(0.0,0.0)};
  NeighborhoodTableEntry e4{4, now - maxAge - 2, now - maxAge - 2, inet::Coord(2.0,0.0), inet::Coord(0.0,0.0)};
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
}

TEST_F(NeighborhoodTableTest, handleMessage) {
  MockNeighborhoodTable mock;
  cMessage* ttl_msg = new cMessage("TitleMessage");
  cMessage* invalid_msg = new cMessage("InvalidMessage");
  simtime_t time = simTime().dbl();
  double maxAge = 1.0;
  // important to define mock spies before invoking the function to test
  EXPECT_CALL(mock, checkTimeToLive())
    .Times(1);
  EXPECT_CALL(mock, scheduleAt(time + maxAge, ttl_msg))
    .Times(1);
  mock.setTitleMessage(ttl_msg);
  mock.setMaxAge(maxAge);
  mock.handleMessage(ttl_msg);
  // test expected exception is thrown
  EXPECT_THROW(mock.handleMessage(invalid_msg), cRuntimeError);
  delete invalid_msg; // suppress weird warning
}

TEST_F(NeighborhoodTableTest, initialize) {
  MockNeighborhoodTable mock;
  simtime_t time = simTime().dbl();
  setSimTime(time);
  double maxAge = 1.0;
  mock.setMaxAge(maxAge);

  cDoubleParImpl* dPar = new cDoubleParImpl();
  dPar->setName("maxAge");
  dPar->setDoubleValue(maxAge);
  mock.addPar(dPar);

  EXPECT_CALL(mock, scheduleAt(time + maxAge, testing::_)).Times(1); // triggers on initialize(0)
  mock.setMaxAge(maxAge);

  mock.initialize(0);
  mock.initialize(1);
  mock.initialize(42);
}
