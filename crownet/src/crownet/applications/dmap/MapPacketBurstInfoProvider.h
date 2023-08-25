/*
 * DpmmCellByteMapper.h
 *
 *  Created on: Aug 23, 2023
 *      Author: vm-sts
 */

#ifndef CROWNET_APPLICATIONS_DMAP_PACKET_DPMMPACKETBYTEMAPPER_H_
#define CROWNET_APPLICATIONS_DMAP_PACKET_DPMMPACKETBYTEMAPPER_H_


#include "crownet/applications/common/BurstInfo.h"
#include "crownet/applications/dmap/dmap_m.h"


namespace crownet {



class MapPacketBurstInfoProvider : public BurstInfoProvider {
public:
    MapPacketBurstInfoProvider(std::shared_ptr<MapHeader> header, std::shared_ptr<MapPacketBase> packet): header(header), packet(packet){
        minPacketSize = this->packetSizeWithCells(1);
    }
    virtual ~MapPacketBurstInfoProvider() = default;

    virtual int getCellCountForBits(inet::b data) const override;
    virtual int getMaxCellCountPerPacket(const inet::b& maxPduSize) const override;
    virtual inet::b getAmoutOfBitsForCell(int numberCells) const override;
    virtual inet::b packetSizeWithCells(int numberCells) const override;
    virtual BurstInfo createBurstInfo(const inet::b& scheduled, const int availableCells, const inet::b& maxPduSize) const override;
    virtual const inet::b& getMinPacketSize() const override { return minPacketSize; }


private:
    std::shared_ptr<MapHeader> header;
    std::shared_ptr<MapPacketBase> packet;
    inet::b minPacketSize;

};


} /* namespace crownet */


#endif /* CROWNET_APPLICATIONS_DMAP_PACKET_DPMMPACKETBYTEMAPPER_H_ */
