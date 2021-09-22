/*
 * BeaconTest.cc
 *
 *  Created on: Sep 21, 2021
 *      Author: vm-sts
 */




#include "crownet/applications/beacon/BeaconReceptionInfo.h"
#include "crownet/applications/beacon/Beacon_m.h"
#include "main_test.h"

using namespace crownet;


class BeaconInfoTest : public BaseOppTest {
 public:
    BeaconInfoTest() {}

    Packet *createAtTime(simtime_t t, int seq, int id=42, int neighours=5, Coord coord={1.2, 3.4}){
        auto oldtime = simTime();
        setSimTime(t);

        const auto& header = makeShared<DynamicBeaconHeader>();
        header->setSequencenumber(seq);
        header->setSourceId(id);
        // mod 32
        uint32_t time = (uint32_t)(simTime().inUnit(SimTimeUnit::SIMTIME_MS) & ((uint64_t)(1) << 32)-1);
        header->setTimestamp(time);


        const auto &beacon = makeShared<DynamicBeaconPacket>();
        beacon->setPos(coord);
        beacon->setEpsilon({0.0, 0.0});
        // measurement time is same as packet creation.
        beacon->setPosTimestamp(time);
        beacon->setNumberOfNeighbours(neighours);
        auto packet = new Packet();
        packet->insertAtFront(header);
        packet->insertAtBack(beacon);

        setSimTime(oldtime);
        return packet;
    }

    void process(BeaconReceptionInfo& info, Packet* pkt, int id=50, simtime_t time=simtime_t::ZERO){
        if (time == simtime_t::ZERO){
            time = simTime();
        }
        info.processInbound(pkt, id, time);
        delete pkt;
    }
};




TEST_F(BeaconInfoTest, BeaconRcvJitter) {

    auto packet = createAtTime(1.0, 1);

    setSimTime(1.030); // + 30ms

    BeaconReceptionInfo info;
    auto id = packet->peekAtFront<DynamicBeaconHeader>()->getSourceId();
    info.setNodeId(id);
    info.processInbound(packet, 55, simTime());

    EXPECT_EQ(info.getInitialSequencenumber(), 1);
    EXPECT_EQ(info.getMaxSequencenumber(), 1);
    EXPECT_EQ(info.getSequencecycle(), 0);
    EXPECT_EQ(info.getPacketsReceivedCount(), 1);
    EXPECT_EQ(info.getPacketsLossCount(), 0);

    EXPECT_EQ(info.getJitter().dbl(), 0.030/16);

    EXPECT_EQ(info.getSentTimePrio(), 1000); //ms
    EXPECT_EQ(info.getReceivedTimePrio(), simTime());
    delete packet;

    //////// second packet

    packet = createAtTime(1.500, 3);


    setSimTime(1.520); // + 20ms

    info.processInbound(packet, 55, simTime());

    EXPECT_EQ(info.getInitialSequencenumber(), 1);
    EXPECT_EQ(info.getMaxSequencenumber(), 3);
    EXPECT_EQ(info.getSequencecycle(), 0);
    EXPECT_EQ(info.getPacketsReceivedCount(), 2);
    EXPECT_EQ(info.getPacketsLossCount(), 1);

    auto jitter = SimTime(0.030/16);
    jitter = jitter + (SimTime(0.01) - jitter)/16;
    EXPECT_EQ(info.getJitter().dbl(), jitter.dbl() );

    EXPECT_EQ(info.getSentTimePrio(), 1500); //ms
    EXPECT_EQ(info.getReceivedTimePrio(), simTime());
    delete packet;
}

TEST_F(BeaconInfoTest, BeaconRcvSequenceWrap1) {
    // 0xFFFD -> 0xFFFE -> 0xFFFF --cycle+1--> 0x0000 -> 0x0001 -> ...
    uint16_t start = 0xFFFC; // 0xFFFF-3
    BeaconReceptionInfo info;
    EXPECT_EQ(info.getSequencecycle(), 0);
    EXPECT_EQ(info.getPacketsReceivedCount(), 0);
    EXPECT_EQ(info.getMaxSequencenumber(), 0);
    for(int i=0; i < 7; i++){
        auto pkt = createAtTime(1.0*i, ++start);
        setSimTime(1.0*i + 0.020);
        info.processInbound(pkt, 30, simTime());
        delete pkt;
    }
    EXPECT_EQ(info.getSequencecycle(), 1<<16);
    EXPECT_EQ(info.getPacketsReceivedCount(), 7);
    EXPECT_EQ(info.getMaxSequencenumber(), 3);
}

TEST_F(BeaconInfoTest, BeaconRcvSequenceWrap2) {

    /**
     *  Case 1: packet.seq > info.maxSeq
     *  a: normal case with sequnce number more than 0x10 away from start or end
     *
     *  Like above but current max value *equals* 0x10.
     *  --> just increases max sequence number and possible losses.
     */

    BeaconReceptionInfo info;
    auto pkt = createAtTime(1.0, 0x10);
    process(info, pkt);
    EXPECT_EQ(info.getMaxSequencenumber(), 0x10);
    EXPECT_EQ(info.getSequencecycle(), 0);
    EXPECT_EQ(info.getPacketsReceivedCount(), 1);

    // old packet before
    pkt = createAtTime(1.0, 0xFFEF+1); // seq = 0xFFF0
    process(info, pkt);
    EXPECT_EQ(info.getMaxSequencenumber(), 0xFFEF+1); // increased!
    EXPECT_EQ(info.getSequencecycle(), 0);
    EXPECT_EQ(info.getPacketsReceivedCount(), 2);

}


TEST_F(BeaconInfoTest, BeaconRcvSequenceWrap3) {
    /**
     *  Case 1: packet.seq > info.maxSeq
     *  b: shortly **after** sequence wrap --> old packet ignore it
     *
     *  If current max sequence number is smaller than 0x10,
     *  assume that received packets with a sequence number greater than
     *  0xFFEF are old packet which will be ignored
     */

    BeaconReceptionInfo info;
    auto pkt = createAtTime(1.0, 0x10-1); // seq = 10
    process(info, pkt);
    EXPECT_EQ(info.getMaxSequencenumber(), 0x10-1);
    EXPECT_EQ(info.getSequencecycle(), 0);
    EXPECT_EQ(info.getPacketsReceivedCount(), 1);

    // old packet before
    pkt = createAtTime(1.0, 0xFFEF+1); // seq = 0xFFF0 (old packet from before wrap)
    process(info, pkt);
    EXPECT_EQ(info.getMaxSequencenumber(), 0x10-1); // same as before
    EXPECT_EQ(info.getSequencecycle(), 0);
    EXPECT_EQ(info.getPacketsReceivedCount(), 2);

}



TEST_F(BeaconInfoTest, BeaconRcvSequenceWrap4) {

    /**
     *  Case 2: packet.seq < info.maxSeq
     *  a: normal case this means an old packet arrived. --> ignore it
     *
     *
     */

    BeaconReceptionInfo info;
    auto pkt = createAtTime(1.0, 0xFFEF - 1);
    process(info, pkt);
    EXPECT_EQ(info.getMaxSequencenumber(), 0xFFEF-1);
    EXPECT_EQ(info.getSequencecycle(), 0);
    EXPECT_EQ(info.getPacketsReceivedCount(), 1);

    // old packet before
    pkt = createAtTime(1.0, 0x10 -1); // seq = 0xFFF0
    process(info, pkt);
    EXPECT_EQ(info.getMaxSequencenumber(), 0xFFEF-1); // same as before packet ignored.
    EXPECT_EQ(info.getSequencecycle(), 0);
    EXPECT_EQ(info.getPacketsReceivedCount(), 2);
}

TEST_F(BeaconInfoTest, BeaconRcvSequenceWrap5) {

    /**
     *  Case 2: packet.seq < info.maxSeq
     *  b: sequence numbers are within 0x10 of start/end of number range --> sequence wrap detected.
     *
     *
     */

    BeaconReceptionInfo info;
    auto pkt = createAtTime(1.0, 0xFFEF +1);
    process(info, pkt);
    EXPECT_EQ(info.getMaxSequencenumber(), 0xFFEF +1);
    EXPECT_EQ(info.getSequencecycle(), 0);
    EXPECT_EQ(info.getPacketsReceivedCount(), 1);

    // old packet before
    pkt = createAtTime(1.0, 0x10 -1); // seq = 0xFFF0
    process(info, pkt);
    EXPECT_EQ(info.getMaxSequencenumber(), 0x10-1); // new one
    EXPECT_EQ(info.getSequencecycle(), 1 << 16);
    EXPECT_EQ(info.getPacketsReceivedCount(), 2);
}




