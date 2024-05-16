/*
 * MapPacketBurstInfoProviderTest.cc
 *
 *  Created on: Aug 24, 2023
 *      Author: vm-sts
 */




#include <functional>
#include <memory>
#include <cmath>
#include <string>

#include "main_test.h"
#include "crownet/crownet_testutil.h"

#include "crownet/applications/dmap/MapPacketBurstInfoProvider.h"

struct TestObject {
    std::shared_ptr<MapHeader> header;
    std::shared_ptr<MapPacketBase> packet;
    std::shared_ptr<BurstInfoProvider> p;
    std::string objName;
};

class MapPacketBurstInfoProviderTest : public BaseOppTestWithParameters<const char*> {
public:


    TestObject get(const char* i) {
        TestObject o;
        if (std::string(i) == "sparse"){
            o.header = std::make_shared<MapHeader>();
            o.packet = std::make_shared<SparseMapPacket>();
            o.p = std::make_shared<MapPacketBurstInfoProvider>(o.header, o.packet);
        } else {
            o.header = std::make_shared<MapHeader>();
            o.packet = std::make_shared<SparseMapPacketWithSharingDomainId>();
            o.p = std::make_shared<MapPacketBurstInfoProvider>(o.header, o.packet);
        }
        return o;
    }

};


constexpr const char* kPets[] = {"sparse", "sparseRsd"};
INSTANTIATE_TEST_SUITE_P(PktType, MapPacketBurstInfoProviderTest, testing::ValuesIn(kPets));

TEST_P(MapPacketBurstInfoProviderTest, getMaxCellCountPerPacket){
    TestObject o = get(GetParam());
    int max = std::floor((inet::b(1400*8)-o.header->getChunkLength()).get() / o.packet->getCellSize().get());
    EXPECT_EQ(o.p->getMaxCellCountPerPacket(inet::b(1400*8)),
           max);

}

TEST_P(MapPacketBurstInfoProviderTest, MinPacketSize) {
   TestObject o = get(GetParam());
   EXPECT_EQ(o.p->getMinPacketSize(),
           o.header->getChunkLength() + o.packet->getCellSize()) << "Packet length must be header + 1 cell";
}


TEST_P(MapPacketBurstInfoProviderTest, packetSize) {
   TestObject o = get(GetParam());
   EXPECT_EQ(o.p->packetSizeWithCells(10),
           o.header->getChunkLength() + o.packet->getCellSize()*10) << "Packet length must be header + 10*1 cell";

   EXPECT_EQ(o.p->packetSizeWithCells(0),
           o.header->getChunkLength() + o.packet->getCellSize()*0) << "Packet length must be header + 0*1 cell";

}

TEST_P(MapPacketBurstInfoProviderTest, getCellCountForBits) {
   TestObject o = get(GetParam());
   auto ret = o.p->getCellCountForBits(inet::b(800));
   EXPECT_EQ(ret, std::floor(inet::b(800).get()/o.packet->getCellSize().get()));

}


TEST_P(MapPacketBurstInfoProviderTest, getAmoutOfBitsForCell) {
   TestObject o = get(GetParam());
   auto ret = o.p->getAmoutOfBitsForCell(13);
   EXPECT_EQ(ret, o.packet->getCellSize()*13);

}


TEST_P(MapPacketBurstInfoProviderTest, createBurstInfo_one_packet_not_full_scheduled_smaller_than_available) {
   TestObject o = get(GetParam());
   inet::b maxPdu(1400*8);
   int availbeCells = 10000; // Infinite will not be toped.
   int maxCellsPerPacket = o.p->getMaxCellCountPerPacket(maxPdu);
   inet::b scheduled = o.p->packetSizeWithCells(maxCellsPerPacket-5); // packet with MAX_Size - 5 cells

   auto ret = o.p->createBurstInfo(scheduled, availbeCells, maxPdu); // m
   EXPECT_EQ(ret.pkt_count,  1);
   EXPECT_EQ(ret.burst_size,  scheduled);

}


TEST_P(MapPacketBurstInfoProviderTest, createBurstInfo_one_packet_not_full_scheduled_more_than_available) {
   TestObject o = get(GetParam());
   inet::b maxPdu(1400*8);
   int maxCellsPerPacket = o.p->getMaxCellCountPerPacket(maxPdu);
   inet::b scheduled = o.p->packetSizeWithCells(maxCellsPerPacket-5); // packet with MAX_Size - 5 cells

   auto ret = o.p->createBurstInfo(scheduled, (maxCellsPerPacket-5)-2, maxPdu); // m

   //
   EXPECT_EQ(ret.pkt_count,  1);
   EXPECT_EQ(ret.burst_size,  scheduled-o.packet->getCellSize() * 2) << "only one packet with burst size smaller than scheduled by 2 cellSizes.";

}

/**
 * The number of packets in a burst as well as the burst_size can only increase in finite values.
 * These are the cellSize and Header+cellSize*1 (aka. smallest pdu). Here where check that the
 * Burst size  stays constant but the content of the one cell is incremented but values smaller than
 * one cellSize. If this is the case the overall burst_size must not increase. Only when the incremental
 * increase in the scheduled amount reaches a full cellSize the burst_size must increase.
 */
TEST_P(MapPacketBurstInfoProviderTest, createBurstInfo_one_packet_not_full_increment_scheduled_amount_to_see_burst_change) {
   TestObject o = get(GetParam());
   inet::b maxPdu(1400*8);
   int availbeCells = 10000; // Infinite will not be toped.
   int maxCellsPerPacket = o.p->getMaxCellCountPerPacket(maxPdu);
   inet::b scheduled = o.p->packetSizeWithCells(maxCellsPerPacket-5); // packet with MAX_Size - 5 cells

   // 1 Expect cells
   auto ret = o.p->createBurstInfo(scheduled, availbeCells, maxPdu); // assume unlimited cells available

   EXPECT_EQ(ret.pkt_count,  1);
   EXPECT_EQ(ret.burst_size,  scheduled) << "only one packet with burst size smaller than scheduled by 2 cellSizes.";

   // 2 Expect still cells  as scheduled is one bit shye of 'cells +1'
   auto scheduled_new = scheduled + o.packet->getCellSize() - inet::b(1); // add almost enough bits to append and additional cell.

   ret = o.p->createBurstInfo(scheduled_new, availbeCells, maxPdu); // assume unlimited cells available

   EXPECT_EQ(ret.pkt_count,  1);
   EXPECT_EQ(ret.burst_size,  scheduled) << "should not change to previous call";

   // 3 Expect cells+1  as scheduled is now corrected by one bit
   auto scheduled_new2 = scheduled_new + inet::b(1); // exaclty one cell more available

   ret = o.p->createBurstInfo(scheduled_new2, availbeCells, maxPdu); // assume unlimited cells available

   EXPECT_EQ(ret.pkt_count,  1);
   EXPECT_EQ(ret.burst_size,  scheduled_new2) << "should schedule one cell more";

   // 3 Expect still cells+1  as scheduled is now at 'cells +1cell +3bit', which is not enough for cells +2
   auto scheduled_new3 = scheduled_new + inet::b(3); // exaclty one cell more available

   ret = o.p->createBurstInfo(scheduled_new3, availbeCells, maxPdu); // assume unlimited cells available

   EXPECT_EQ(ret.pkt_count,  1);
   EXPECT_EQ(ret.burst_size,  scheduled_new2) << "should be the same scheduled_new2";

}

TEST_P(MapPacketBurstInfoProviderTest, createBurstInfo_test_packet_count_increase) {
    TestObject o = get(GetParam());
    inet::b maxPdu(1400*8);
    int availbeCells = 10000; // Infinite will not be toped.
    int maxCellsPerPacket = o.p->getMaxCellCountPerPacket(maxPdu);
    inet::b scheduled = o.p->packetSizeWithCells(maxCellsPerPacket-1); // packet with one cell slot empty

    auto ret = o.p->createBurstInfo(scheduled, availbeCells, maxPdu);
    EXPECT_EQ(ret.pkt_count,  1);
    EXPECT_EQ(ret.burst_size,  scheduled);


    // creep up to max packet size
    inet::b scheduled_one_bit_below_of_two_packets = o.p->packetSizeWithCells(maxCellsPerPacket) - inet::b(1);
    ret = o.p->createBurstInfo(scheduled_one_bit_below_of_two_packets, availbeCells, maxPdu);
    EXPECT_EQ(ret.pkt_count,  1);
    EXPECT_EQ(ret.burst_size,  scheduled) << "should be the same scheduled";


    // go slightly over max packet size (by 1 bit) but less than cellSize (one packet but full)
    inet::b scheduled_one_bit_below_of_two_packets2 = o.p->packetSizeWithCells(maxCellsPerPacket) + inet::b(1);
    ret = o.p->createBurstInfo(scheduled_one_bit_below_of_two_packets2, availbeCells, maxPdu);
    EXPECT_EQ(ret.pkt_count,  1);
    EXPECT_EQ(ret.burst_size,  o.p->packetSizeWithCells(maxCellsPerPacket)) << "should be full sized packet but only one";

    // go slightly over max packet size but less than minPDU (still just one full packet)
    inet::b scheduled_more_than_one_pkt_less_than_min_pdu = o.p->packetSizeWithCells(maxCellsPerPacket) + o.p->getMinPacketSize() - inet::b(1);
    ret = o.p->createBurstInfo(scheduled_more_than_one_pkt_less_than_min_pdu, availbeCells, maxPdu);
    EXPECT_EQ(ret.pkt_count,  1);
    EXPECT_EQ(ret.burst_size,  o.p->packetSizeWithCells(maxCellsPerPacket)) << "should be full sized packet but only one";


    // go to max packet + minPDU (now 2 packets)
    inet::b scheduled_more_than_one_pkt = o.p->packetSizeWithCells(maxCellsPerPacket) + o.p->getMinPacketSize();
    ret = o.p->createBurstInfo(scheduled_more_than_one_pkt, availbeCells, maxPdu);
    EXPECT_EQ(ret.pkt_count,  2);
    EXPECT_EQ(ret.burst_size,  scheduled_more_than_one_pkt) << "should be full sized + minSize";


    // go to max packet + minPDU + 1 bit(now 2 packets but same as last test)
    inet::b scheduled_more_than_one_pkt_2 = o.p->packetSizeWithCells(maxCellsPerPacket) + o.p->getMinPacketSize() + inet::b(1);
    ret = o.p->createBurstInfo(scheduled_more_than_one_pkt_2, availbeCells, maxPdu);
    EXPECT_EQ(ret.pkt_count,  2);
    EXPECT_EQ(ret.burst_size,  scheduled_more_than_one_pkt) << "should be full sized + minSize";
}

TEST_P(MapPacketBurstInfoProviderTest, createBurstInfo_test_zero_burst_0) {
    TestObject o = get(GetParam());
    inet::b maxPdu(1400*8);

    auto ret = o.p->createBurstInfo(inet::b(0), 10000, maxPdu);
    EXPECT_EQ(ret.pkt_count,  0);
    EXPECT_EQ(ret.burst_size,  inet::b(0)) << "should be zero!";
}

TEST_P(MapPacketBurstInfoProviderTest, createBurstInfo_test_zero_burst_1) {
    TestObject o = get(GetParam());
    inet::b maxPdu(1400*8);

    auto ret = o.p->createBurstInfo(o.p->getMinPacketSize() - inet::b(1), 10000, maxPdu);
    EXPECT_EQ(ret.pkt_count,  0);
    EXPECT_EQ(ret.burst_size,  inet::b(0)) << "should be zero!";

    ret = o.p->createBurstInfo(o.p->getMinPacketSize(), 10000, maxPdu);
    EXPECT_EQ(ret.pkt_count,  1);
    EXPECT_EQ(ret.burst_size,  o.p->getMinPacketSize());
}

TEST_P(MapPacketBurstInfoProviderTest, createBurstInfo_test_zero_burst_2) {
    TestObject o = get(GetParam());
    inet::b maxPdu(1400*8);

    auto ret = o.p->createBurstInfo(inet::b(100000), 0, maxPdu);
    EXPECT_EQ(ret.pkt_count,  0);
    EXPECT_EQ(ret.burst_size,  inet::b(0)) << "should be zero!";
}


TEST_P(MapPacketBurstInfoProviderTest, createBurstInfo_test_with_infinet_scheduled) {
    TestObject o = get(GetParam());
    inet::b maxPdu(1400*8);
    int maxCellsPerPacket = o.p->getMaxCellCountPerPacket(maxPdu);

    auto ret = o.p->createBurstInfo(inet::b(100000), 1, maxPdu);
    EXPECT_EQ(ret.pkt_count,  1);
    EXPECT_EQ(ret.burst_size,  o.p->getMinPacketSize());

    ret = o.p->createBurstInfo(inet::b(100000), 2, maxPdu);
    EXPECT_EQ(ret.pkt_count,  1);
    EXPECT_EQ(ret.burst_size,  o.p->getMinPacketSize() + o.packet->getCellSize());

    ret = o.p->createBurstInfo(inet::b(100000), maxCellsPerPacket-1, maxPdu);
    EXPECT_EQ(ret.pkt_count,  1);
    EXPECT_EQ(ret.burst_size,  o.p->getMinPacketSize() + o.packet->getCellSize()*(maxCellsPerPacket-2));

    ret = o.p->createBurstInfo(inet::b(100000), maxCellsPerPacket, maxPdu);
    EXPECT_EQ(ret.pkt_count,  1);
    EXPECT_EQ(ret.burst_size,  o.p->getMinPacketSize() + o.packet->getCellSize()*(maxCellsPerPacket-1));

    ret = o.p->createBurstInfo(inet::b(100000), maxCellsPerPacket+1, maxPdu);
    EXPECT_EQ(ret.pkt_count,  2);
    EXPECT_EQ(ret.burst_size,  o.p->getMinPacketSize() + o.p->packetSizeWithCells(maxCellsPerPacket));

}


//todo: test with limited availbeCells
//todo: update other test with setup used in  createBurstInfo_test_packet_count_increase
