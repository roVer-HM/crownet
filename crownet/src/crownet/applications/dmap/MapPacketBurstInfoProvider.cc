/*
 * DpmmCellByteMapper.cc
 *
 *  Created on: Aug 23, 2023
 *      Author: vm-sts
 */

#include "crownet/applications/dmap/MapPacketBurstInfoProvider.h"

namespace crownet {


int MapPacketBurstInfoProvider::getCellCountForBits(inet::b data) const {
    return std::floor(data.get() / this->packet->getCellSize().get());
}

int MapPacketBurstInfoProvider::getMaxCellCountPerPacket(const inet::b& maxPduSize) const {
    return  (int)std::floor((maxPduSize - this->header->getChunkLength()).get() / this->packet->getCellSize().get());
}

inet::b MapPacketBurstInfoProvider::getAmoutOfBitsForCell(int numberCells) const {
    return inet::b(numberCells * this->packet->getCellSize().get());
}

inet::b MapPacketBurstInfoProvider::packetSizeWithCells(int numberCells) const {
    return this->header->getChunkLength() + this->packet->getCellSize()*numberCells;
}

BurstInfo MapPacketBurstInfoProvider::createBurstInfo(const inet::b& scheduled, const int availableCells, const inet::b& maxPduSize) const {

    inet::b cellSize =  this->packet->getCellSize();
    inet::b headerSize = this->header->getChunkLength();
    int maxNumberCellsPerPacket = (int)std::floor((maxPduSize - headerSize).get() / cellSize.get());

    // actual size of packet with maximal number of cells. (Will be smaller than maxPduSize due to alignment)
    inet::b maxPacketSize = this->packetSizeWithCells(maxNumberCellsPerPacket);

    // number packets for availableCells
    int _full_packets_for_available = std::floor(availableCells/maxNumberCellsPerPacket);
    inet::b lastPacketSizeForAvailableData = this->packetSizeWithCells(availableCells - _full_packets_for_available*maxNumberCellsPerPacket);
    int packetsNeededForAvailableData = _full_packets_for_available;
    if (lastPacketSizeForAvailableData >= getMinPacketSize()) {
        // only if the last packet at least contains the header and one cell.
        packetsNeededForAvailableData++;
    } else {
        lastPacketSizeForAvailableData = inet::b(0); // reset last packet size.
    }

    // number packers for scheduled data
    int _full_packets_for_scheduled = std::floor(scheduled.get()  / maxPacketSize.get());
    inet::b lastPacketSizeForScheduledData = scheduled - maxPacketSize *_full_packets_for_scheduled;
    int packetsPossibleToSchedule = _full_packets_for_scheduled;
    if (lastPacketSizeForScheduledData >= getMinPacketSize()) {
        // align lastPacketSize with Header and cells.
        lastPacketSizeForScheduledData = this->packetSizeWithCells( std::floor((lastPacketSizeForScheduledData - headerSize).get() / cellSize.get()) );
        packetsPossibleToSchedule++;
    } else {
        lastPacketSizeForScheduledData = inet::b(0); // rest last packet size.
    }

    BurstInfo b;
    inet::b burst_size;

    if (packetsNeededForAvailableData <= packetsPossibleToSchedule){
        // enough data scheduled to send all. Use *AvailableData
        b.pkt_count = packetsNeededForAvailableData;
        b.burst_size =  maxPacketSize * _full_packets_for_available + lastPacketSizeForAvailableData;
    } else {
        // not enough space left. Send as much as possible. Use  *ForScheduledData.
        b.pkt_count = packetsPossibleToSchedule;
        b.burst_size = maxPacketSize*_full_packets_for_scheduled + lastPacketSizeForScheduledData;
    }

    return b;

}


} /* namespace crownet */
