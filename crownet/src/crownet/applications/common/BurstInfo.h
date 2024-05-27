/*
 * BurstInfoProvider.h
 *
 *  Created on: Aug 24, 2023
 *      Author: vm-sts
 */

#ifndef CROWNET_APPLICATIONS_DMAP_PACKET_BURSTINFOPROVIDER_H_
#define CROWNET_APPLICATIONS_DMAP_PACKET_BURSTINFOPROVIDER_H_

//#include "inet/common/INETDefs.h"
#include "inet/common/Units.h"


namespace inet {

using namespace units::values;

}  // namespace inet

namespace crownet {


struct BurstInfo {
    int pkt_count;
    inet::b burst_size;

};

class BurstInfoProvider {
public:
    BurstInfoProvider(){};
    virtual ~BurstInfoProvider() = default;


    /**
     * Creates Burst info of the form {pkt_count, burst_size} where the burst_size must be aligned with cells and header of
     * the packet. The Burst can be limited either by the scheduled amount provided by the scheduler or by the number of
     * valid cells that can be transmitted. Note: What constitutes as `valid` as well as the order of specific cells is
     * out of scope of the BurstInfoProvider. This class only provides the number of packets and the total amount of data
     * handed down the stack.
     *
     * Example: Header 4B (HHHH) CellSize 2B (CC) with a maximum packet data unit (pdu) of 10B a packet can at most contain
     * 4 cells and the header.
     * *-------------*
     * |HHHH.CC.CC.CC|
     * *-------------*
     *
     * Example 1: Scheduler provides 120 bit (15 B) assuming unlimited `availableCells` in the application
     * *-------------*  *-------------*
     * |HHHH.CC.CC.CC|  |HHHH.CC.CC.CC|
     * *-------------*  *-------------*
     *  0           9         ^
     *                        15 B are not enough to create two packets as the second packet needs
     *                        at least 6B to add the header and *one* cell.
     * --> BurstInfo{1, 10B}
     *
     * Example 2:   Scheduler provides 128 bit (16 B) assuming unlimited `availableCells` in the application
     * --> BurstInfo{2, 16B}
     *
     * Example 3: Scheduler provides 136 bit (17 B) assuming unlimited `availableCells` in the application
     * --> BurstInfo{2, 16B} Same as in example 2 as the second cell in the packet would exceed the scheduled amount
     *
     * Example 4: Scheduler provides 144 bit (18B), the application has 2 `availableCells` in the application
     * --> BurstInfo{1, 8B} The scheduler provides us with space for 2 packets and 5 cells (2x4B (header) + 5x2B (cells) = 18B)
     *                      but application has only 2 `availableCells`, thus header + 2 cells = 8B
     *
     */
    virtual BurstInfo createBurstInfo(const inet::b& scheduled, const int availableCells, const inet::b& maxPduSize) const = 0;


    virtual int getCellCountForBits(inet::b data) const = 0;
    virtual inet::b getAmoutOfBitsForCell(int numberCells) const = 0;
    virtual inet::b packetSizeWithCells(int numberCells) const = 0;
    virtual const inet::b& getMinPacketSize() const = 0;
    virtual int getMaxCellCountPerPacket(const inet::b& maxPduSize) const = 0;


};

} //namespace


#endif /* CROWNET_APPLICATIONS_DMAP_PACKET_BURSTINFOPROVIDER_H_ */
