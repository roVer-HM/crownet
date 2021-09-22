
#include "crownet/applications/beacon/BeaconReceptionInfo.h"
#include "main_test.h"

using namespace crownet;

class BeaconReceptionInfoTest : public BaseOppTest {
 public:
    BeaconReceptionInfoTest() {}

    BeaconReceptionInfo build(int id, int t1, int t2, inet::Coord c1, inet::Coord c2){

        BeaconReceptionInfo info;
        info.setNodeId(id);
        info.setSentTimePrio(t1);
        info.setReceivedTimePrio(t2);
        info.setPos(c1);
        info.setEpsilon(c2);
        return info;
    }
};



TEST_F(BeaconReceptionInfoTest, constructor) {
    int expectedId = 1;
    int expectedTimeSend = 1;
    int expectedTimeReceived = 1;
    inet::Coord expectedPos = inet::Coord(1.0,1.0);
    inet::Coord expectedEpsilon = inet::Coord(1.0,1.0);

    BeaconReceptionInfo e0;
    EXPECT_TRUE(e0.getSentTimePrio() == 0);
    EXPECT_TRUE(e0.getReceivedTimePrio() == 0);
    EXPECT_TRUE(e0.getPos() == inet::Coord(0.0,0.0));
    EXPECT_TRUE(e0.getEpsilon() == inet::Coord(0.0,0.0));

    BeaconReceptionInfo e1 = build(expectedId,expectedTimeSend,expectedTimeReceived,expectedPos,expectedEpsilon);
    EXPECT_TRUE(e1.getNodeId() == expectedId);
    EXPECT_TRUE(e1.getSentTimePrio() == expectedTimeSend);
    EXPECT_TRUE(e1.getReceivedTimePrio() == expectedTimeReceived);
    EXPECT_TRUE(e1.getPos() == expectedPos);
    EXPECT_TRUE(e1.getEpsilon() == expectedEpsilon);
}


TEST_F(BeaconReceptionInfoTest, equals) {
    // also test private function copy()
    int nodeId = 0;
    BeaconReceptionInfo e0 = build (nodeId, 0, 0, inet::Coord(0.0,0.0), inet::Coord(0.0,0.0));
    BeaconReceptionInfo e1 = build (1, 2, 2, inet::Coord(1.0,1.0), inet::Coord(1.0,1.0));

    EXPECT_TRUE(e0.getNodeId() == nodeId);

    EXPECT_FALSE(e0.getNodeId() == e1.getNodeId());
    EXPECT_FALSE(e0.getSentTimePrio() == e1.getSentTimePrio());
    EXPECT_FALSE(e0.getReceivedTimePrio() == e1.getReceivedTimePrio());
    EXPECT_FALSE(e0.getPos() == e1.getPos());
    EXPECT_FALSE(e0.getEpsilon() == e1.getEpsilon());
}

TEST_F(BeaconReceptionInfoTest, string) {
    setSimTime(1.0);

    BeaconReceptionInfo e0 = build(0, 5, 1, inet::Coord(0.0,0.0), inet::Coord(0.0,0.0));
    std::string expected = "{id: 0 a_t:1s age:0s}";
    std::string result = e0.str();
    EXPECT_EQ(result, expected);
}
