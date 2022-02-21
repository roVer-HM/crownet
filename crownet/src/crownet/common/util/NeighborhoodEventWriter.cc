/*
 * BeaconReceptionInfoLogger.cc
 *
 *  Created on: Oct 24, 2021
 *      Author: vm-sts
 */

#include "NeighborhoodEventWriter.h"
#include "crownet/common/converter/OsgCoordinateConverter.h"

namespace crownet {

NeighborhoodEventWriter::NeighborhoodEventWriter(std::string filePath, std::string sep, long bufferSize)
        : BaseFileWriter(filePath, sep, bufferSize){}


void NeighborhoodEventWriter::onInit(){
    write() << "table_owner" << sep << "event" << sep << "event_time" << sep << "receveid_at_time" << sep \
            << "source_node" << sep << "posX" << sep << "posY" << sep \
            << "beacon_value" << sep << "pkt_count" << sep << "pkt_loss" << sep << "cell_x" << sep << "cell_y" << endl;
}

void NeighborhoodEventWriter::neighborhoodEntryRemoved(INeighborhoodTable* table, BeaconReceptionInfo* info){
    writeData(table, info, "ttl_reached");
}

void NeighborhoodEventWriter::neighborhoodEntryPostChanged(INeighborhoodTable* table, BeaconReceptionInfo* info) {
    writeData(table, info, "changed");
}

void NeighborhoodEventWriter::writeData(INeighborhoodTable* table, BeaconReceptionInfo* info, std::string event){
    auto pos = globalMapHandler->getConverter()->position_cast_traci(info->getPos());
    auto cellPos = globalMapHandler->getCellKeyProvider()->getCellPosition(pos);


    write() << table->getOwnerId() << sep << event << sep << simTime().dbl() << sep << info->getReceivedTimePrio() << sep \
            << info->getNodeId() << sep << info->getPos().x << sep \
            << info->getPos().y << sep << info->getBeaconValue() << sep \
            << info->getPacketsReceivedCount() << sep << info->getPacketsLossCount() << sep \
            << cellPos.x << sep << cellPos.y << endl;

}

}
