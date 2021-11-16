/*
 * BeaconReceptionInfoLogger.cc
 *
 *  Created on: Oct 24, 2021
 *      Author: vm-sts
 */

#include "NeighborhoodEventWriter.h"

namespace crownet {

NeighborhoodEventWriter::NeighborhoodEventWriter(std::string filePath, std::string sep, long bufferSize)
        : BaseFileWriter(filePath, sep, bufferSize){}


void NeighborhoodEventWriter::onInit(){
    write() << "table_owner" << sep << "event" << sep << "receveid_at_time" << sep \
            << "source_node" << sep << "posX" << sep << "posY" << sep \
            << "beacon_value" << sep << "pkt_count" << sep << "pkt_loss" << endl;
}

void NeighborhoodEventWriter::neighborhoodEntryRemoved(INeighborhoodTable* table, BeaconReceptionInfo* info){
    writeData(table, info, "ttl_reached");
}

void NeighborhoodEventWriter::neighborhoodEntryPostChanged(INeighborhoodTable* table, BeaconReceptionInfo* info) {
    writeData(table, info, "changed");
}

void NeighborhoodEventWriter::writeData(INeighborhoodTable* table, BeaconReceptionInfo* info, std::string event){
    write() << table->getOwnerId() << sep << event << sep << info->getReceivedTimePrio() << sep \
            << info->getNodeId() << sep << info->getPos().x << sep \
            << info->getPos().y << sep << info->getBeaconValue() << sep \
            << info->getPacketsReceivedCount() << sep << info->getPacketsLossCount() << endl;

}

}
