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

    std::vector<std::string> header = {"table_owner", "event_number", "event", "event_time",
            "received_at_time", "sent_time", "source_node", "posX", "posY",
            "beacon_value", "pkt_count", "pkt_loss", "pkt_seq", "cell_x", "cell_y",
            };

    write() << "#version=3 " << endl;
    for (int i = 0; i < header.size(); i++){
        if (i < header.size() -1){
            write() << header[i] << sep;
        } else {
            write() << header[i] << endl;
        }
    }

//    write() << "table_owner" << sep << "event" << sep << "event_time" << sep << "receveid_at_time" << sep \
//            << "source_node" << sep << "posX" << sep << "posY" << sep \
//            << "beacon_value" << sep << "pkt_count" << sep << "pkt_loss" << sep << "cell_x" << sep << "cell_y" << sep \
//            << "sent_time" << sep << "received_time" << endl;
}

void NeighborhoodEventWriter::neighborhoodEntryRemoved(INeighborhoodTable* table, BeaconReceptionInfo* info){
    writeData(table, info, "ttl_reached");
}

void NeighborhoodEventWriter::neighborhoodEntryPostChanged(INeighborhoodTable* table, BeaconReceptionInfo* info) {
    writeData(table, info, "post_change");
}

void NeighborhoodEventWriter::neighborhoodEntryPreChanged(INeighborhoodTable* table, BeaconReceptionInfo* oldInfo){
    writeData(table, oldInfo, "pre_change");
}

void NeighborhoodEventWriter::neighborhoodEntryDropped(INeighborhoodTable* table, BeaconReceptionInfo* info){
    writeData(table, info, "dropped");
}


void NeighborhoodEventWriter::writeData(INeighborhoodTable* table, BeaconReceptionInfo* info, std::string event){
    auto pos = globalMapHandler->getConverter()->position_cast_traci(info->getPos());
    auto cellPos = globalMapHandler->getCellKeyProvider()->getCellPosition(pos);
    int beaconValue = 0;
    if (event == "post_change"){
        beaconValue = info->getBeaconValue();
    } else if (event == "dropped") {
        beaconValue = 0;
    } else { //pre_changed, ttl_reached
        beaconValue = -1*info->getBeaconValue();
    }
    eventnumber_t event_number = getSimulation()->getEventNumber();

    write() << table->getOwnerId() << sep << event_number << sep << event << sep << simTime().dbl() << sep \
            << info->getReceivedTimeCurrent() << sep << info->getSentSimTimeCurrent() << sep \
            << info->getNodeId() << sep << info->getPos().x << sep << info->getPos().y << sep \
            << beaconValue << sep << info->getPacketsReceivedCount() << sep \
            << info->getPacketsLossCount() << sep << info->getMaxSequencenumber() << sep \
            << cellPos.x << sep << cellPos.y << endl;

}

}
