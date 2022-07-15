/*
 * EmaMeterTest.cc
 *
 *  Created on: Sep 22, 2021
 *      Author: vm-sts
 */





#include "crownet/mobility/BonnMotionMobilityServer.h"
#include "main_test.h"

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


using namespace crownet;


class BonnMotionTest : public BaseOppTest {
 public:
    BonnMotionTest() {}
    void SetUp() override {
        fs::path dir = fs::absolute(__FILE__).parent_path();
        bmFile1.loadFile((dir / "bmFile1.bonnMotion").c_str());
        bmFile2.loadFile((dir / "bmFile2.bonnMotion").c_str());
    }
 protected:
    BonnMotionServerFile bmFile1;
    BonnMotionServerFile bmFile2;
};


TEST_F(BonnMotionTest, peekTest){

    auto t1 = bmFile1.peekAtNextTimeLineIndex();
    EXPECT_EQ(t1, std::make_pair(0, simtime_t(0.4)));

    auto t2 = bmFile2.peekAtNextTimeLineIndex();
    EXPECT_EQ(t2, std::make_pair(2, simtime_t(0.2)));
}

TEST_F(BonnMotionTest, getNextTest){

    setSimTime(0.2);
    bmFile2.getNextTimeLineIndex(simTime());

    setSimTime(0.4);
    EXPECT_TRUE(bmFile1.hasTraceForTime(simTime()));
    bmFile1.getNextTimeLineIndex(simTime());
    EXPECT_FALSE(bmFile1.hasTraceForTime(simTime()));

    EXPECT_TRUE(bmFile2.hasTraceForTime(simTime()));
    bmFile2.getNextTimeLineIndex(simTime());
    EXPECT_TRUE(bmFile2.hasTraceForTime(simTime()));

}



