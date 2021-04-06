#include "main_test.h"
#include "crownet/common/NeighborhoodTableEntry.h"

using namespace crownet;

class NeighborhoodTableEntryTest : public BaseOppTest {
 public:
    NeighborhoodTableEntryTest() {}
};

TEST_F(NeighborhoodTableEntryTest, constructor) {
    int expectedId = 1;
    int expectedTimeSend = 1;
    int expectedTimeReceived = 1;
    inet::Coord expectedPos = inet::Coord(1.0,1.0);
    inet::Coord expectedEpsilon = inet::Coord(1.0,1.0);

    NeighborhoodTableEntry e0;
    EXPECT_TRUE(e0.getTimeSend() == 0);
    EXPECT_TRUE(e0.getTimeReceived() == 0);
    EXPECT_TRUE(e0.getPos() == inet::Coord(0.0,0.0));
    EXPECT_TRUE(e0.getEpsilon() == inet::Coord(0.0,0.0));

    NeighborhoodTableEntry e1{expectedId,expectedTimeSend,expectedTimeReceived,expectedPos,expectedEpsilon};
    EXPECT_TRUE(e1.getNodeId() == expectedId);
    EXPECT_TRUE(e1.getTimeSend() == expectedTimeSend);
    EXPECT_TRUE(e1.getTimeReceived() == expectedTimeReceived);
    EXPECT_TRUE(e1.getPos() == expectedPos);
    EXPECT_TRUE(e1.getEpsilon() == expectedEpsilon);

    NeighborhoodTableEntry e2{expectedId,expectedTimeSend,expectedPos,expectedEpsilon};
    EXPECT_TRUE(e2.getNodeId() == expectedId);
    EXPECT_TRUE(e2.getTimeSend() == expectedTimeSend);
    EXPECT_TRUE(e2.getTimeReceived() == 0);
    EXPECT_TRUE(e2.getPos() == expectedPos);
    EXPECT_TRUE(e2.getEpsilon() == expectedEpsilon);

    NeighborhoodTableEntry e3{expectedId,expectedTimeSend,expectedTimeReceived,expectedPos};
    EXPECT_TRUE(e3.getNodeId() == expectedId);
    EXPECT_TRUE(e3.getTimeSend() == expectedTimeSend);
    EXPECT_TRUE(e3.getTimeReceived() == expectedTimeReceived);
    EXPECT_TRUE(e3.getPos() == expectedPos);
    EXPECT_TRUE(e3.getEpsilon() == inet::Coord(0.0,0.0));

    NeighborhoodTableEntry e4{expectedId,expectedTimeSend,expectedPos};
    EXPECT_TRUE(e4.getNodeId() == expectedId);
    EXPECT_TRUE(e4.getTimeSend() == expectedTimeSend);
    EXPECT_TRUE(e4.getTimeReceived() == 0);
    EXPECT_TRUE(e4.getPos() == expectedPos);
    EXPECT_TRUE(e4.getEpsilon() == inet::Coord(0.0,0.0));
}

TEST_F(NeighborhoodTableEntryTest, print) {
    NeighborhoodTableEntry e0{0, 1, 1, inet::Coord(0.0,0.0), inet::Coord(0.0,0.0)};
    std::stringstream ss;
    ss << e0;
    std::string result = ss.str();
    std::string expected = "{id: 0 tSend: 1 tReceived: 1 pos: (0, 0, 0) m epsilon: (0, 0, 0) m";
    EXPECT_TRUE(result.compare(expected) == 0);
}

TEST_F(NeighborhoodTableEntryTest, equals) {
    // also test private function copy()
    int nodeId = 0;
    NeighborhoodTableEntry e0{nodeId, 0, 0, inet::Coord(0.0,0.0), inet::Coord(0.0,0.0)};
    NeighborhoodTableEntry e1{1, 2, 2, inet::Coord(1.0,1.0), inet::Coord(1.0,1.0)};

    e0 = e0;
    EXPECT_TRUE(e0.getNodeId() == nodeId);

    e0 = e1;
    EXPECT_TRUE(e0.getNodeId() == e1.getNodeId());
    EXPECT_TRUE(e0.getTimeSend() == e1.getTimeSend());
    EXPECT_TRUE(e0.getTimeReceived() == e1.getTimeReceived());
    EXPECT_TRUE(e0.getPos() == e1.getPos());
    EXPECT_TRUE(e0.getEpsilon() == e1.getEpsilon());
}

TEST_F(NeighborhoodTableEntryTest, string) {
    NeighborhoodTableEntry e0{0, 1, 1, inet::Coord(0.0,0.0), inet::Coord(0.0,0.0)};
    std::string expected = "{id: 0 tSend: 1 tReceived: 1 pos: (0, 0, 0) m epsilon: (0, 0, 0) m";
    std::string result = e0.str();
    EXPECT_TRUE(result.compare(expected) == 0);
}
