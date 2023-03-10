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

    write() << "#version=4 " << endl;
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

void NeighborhoodEventWriter::neighborhoodEntryDropped(INeighborhoodTable* table, BeaconReceptionInfo* info){
    writeData(table, info, "dropped");
}


void NeighborhoodEventWriter::neighborhoodEntryLeaveCell(INeighborhoodTable* table, BeaconReceptionInfo* info){
    writeData(table, info, "leave_cell");
}

void NeighborhoodEventWriter::neighborhoodEntryEnterCell(INeighborhoodTable* table, BeaconReceptionInfo* info){
    writeData(table, info, "enter_cell");
}

void NeighborhoodEventWriter::neighborhoodEntryStayInCell(INeighborhoodTable* table, BeaconReceptionInfo* info){
    writeData(table, info, "stay_in_cell");
}



void NeighborhoodEventWriter::writeData(INeighborhoodTable* table, BeaconReceptionInfo* info, std::string event){
    int beaconValue = 0;
    simtime_t rcvdTime;
    simtime_t sentTime;
    Coord position;
    auto currentData = info->getCurrentData();
    auto prioData = info->getPrioData();
    if (event == "dropped") {
        beaconValue = 0;
    } else if (event == "leave_cell"){
        // current time but old position
        beaconValue = -1;
        rcvdTime = currentData->getReceivedTime();
        sentTime = currentData->getCreationTime();
        position = prioData->getPosition();
    } else if (event == "enter_cell") {
        // current time and current position
        beaconValue = 1;
        rcvdTime = currentData->getReceivedTime();
        sentTime = currentData->getCreationTime();
        position = currentData->getPosition();
    } else if (event == "stay_in_cell") {
        // current time and current position no change of beacon value
        beaconValue = 0;
        rcvdTime = currentData->getReceivedTime();
        sentTime = currentData->getCreationTime();
        position = currentData->getPosition();
    } else if (event == "ttl_reached"){
        // current time and current position (last known values but older than TTL)
        beaconValue = -1;
        rcvdTime = currentData->getReceivedTime();
        sentTime = currentData->getCreationTime();
        position = currentData->getPosition();
    } else { //pre_changed, ttl_reached
        throw cRuntimeError("wrong event: %s", event.c_str());
    }
    eventnumber_t event_number = getSimulation()->getEventNumber();
    auto traciPosition = globalMapHandler->getConverter()->position_cast_traci(position);
    auto cellPos = globalMapHandler->getCellKeyProvider()->getCellPosition(traciPosition);

    write() << table->getOwnerId() << sep << event_number << sep << event << sep << simTime().dbl() << sep \
            << rcvdTime << sep << sentTime << sep << info->getNodeId() << sep << position.x << sep << position.y << sep \
            << beaconValue << sep << info->getPacketsReceivedCount() << sep << info->getPacketsLossCount() << sep \
            << info->getMaxSequenceNumber() << sep << cellPos.x << sep << cellPos.y << endl;
}

}
