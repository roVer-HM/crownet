/*
 * BeaconReceptionInfoLogger.h
 *
 *  Created on: Oct 24, 2021
 *      Author: vm-sts
 */

#pragma once
#include "crownet/common/util/FileWriter.h"
#include "crownet/neighbourhood/contract/INeighborhoodTable.h"
#include <fstream>


namespace crownet {

class NeighborhoodEventWriter : public NeighborhoodEntryListner, public BaseFileWriter
{
public:
    NeighborhoodEventWriter(std::string filePath="", std::string sep = ";", long bufferSize=8192);

    virtual ~NeighborhoodEventWriter() = default;

    virtual void onInit() override;

    virtual void neighborhoodEntryPreChanged(INeighborhoodTable* table, BeaconReceptionInfo* oldInfo) override {/* do nothing */}
    virtual void neighborhoodEntryPostChanged(INeighborhoodTable* table, BeaconReceptionInfo* info) override;
    virtual void neighborhoodEntryRemoved(INeighborhoodTable* table, BeaconReceptionInfo* info) override;

    void writeData(INeighborhoodTable* table, BeaconReceptionInfo* info, std::string event);

private :

    void init();
};

}



