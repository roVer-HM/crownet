#include "main_test.h"
#include "crownet/common/NeighborhoodTable.h"

#include <gmock/gmock.h>

using namespace crownet;

class MockNeighborhoodTable : public NeighborhoodTable{

  public:
//    MOCK_METHOD(checkTimeToLive,void());
    // (New) MOCK_METHOD(double, Bar, (std::string s), (override));
    MOCK_METHOD(void, checkTimeToLive,(),(override));
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
  NeighborhoodTable nTable;
//  mock.handleMessage(msg);
//  cMessage* msg = new cMessage("TitleMessage");
//  nTable.setTitleMessage(msg);
//  nTable.handleMessage(msg);
//  EXPECT_CALL(mock,checkTimeToLive()).Times(1);
  EXPECT_TRUE(1==1);
}

//NeighborhoodTable::~NeighborhoodTable(){
//    if (ttl_msg != nullptr)
//        cancelAndDelete(ttl_msg);
//}
//
//// cSimpleModule
//void NeighborhoodTable::initialize(int stage){
//    cSimpleModule::initialize(stage);
//    if (stage == INITSTAGE_LOCAL){
//        maxAge = par("maxAge");
//        ttl_msg = new cMessage("NeighborhoodTable_ttl");
//        scheduleAt(simTime() + maxAge, ttl_msg);
//        WATCH_MAP(_table);
//        WATCH(maxAge);
//    }
//}
//
//void NeighborhoodTable::handleMessage(cMessage *msg){
//    if (msg == ttl_msg){
//        checkTimeToLive();
//        scheduleAt(simTime() + maxAge, ttl_msg);
//    } else {
//        throw cRuntimeError("Unknown Message received");
//    }
//}
