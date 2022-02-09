/*
 * EmaMeterTest.cc
 *
 *  Created on: Sep 22, 2021
 *      Author: vm-sts
 */





#include "crownet/applications/beacon/BeaconReceptionInfo.h"
#include "crownet/applications/beacon/Beacon_m.h"
#include "main_test.h"

#include "crownet/queueing/meter/EmaPacketMeter.h"

using namespace crownet;


class EmaMeterTest : public BaseOppTest {
 public:
    EmaMeterTest() {}
};


TEST_F(EmaMeterTest, ConstTimeInterval){

    EmaPacketMeter m;
    m.setAlpha(1./16.);

    Packet* pkt;
    for(int i=0; i < 100; i++){
        setSimTime(i*1.0);
        pkt = build(b(300));
        m.meterPacket(pkt);
        delete pkt;
    }
    auto stats = m.getStats();
    EXPECT_EQ(stats.getPacketCount(), 100-1);
    EXPECT_EQ(stats.getTotalData().get(), 100*300 - 300); // offset of 300b due to possibility of multiple packet per time point
    // PacketMeter does not count the first packet correctly.
    // Compare with 99 values instead of 100 values
    EXPECT_NEAR(stats.getDataRate().get(), 300.0, 0.001);
    EXPECT_NEAR(stats.getPacketRate(), 1.0, 0.0001);
}


