/*
 * BeaconReceptionInfoLogger.h
 *
 *  Created on: Oct 24, 2021
 *      Author: vm-sts
 */

#pragma once
#include "crownet/common/util/FileWriter.h"
#include "crownet/common/IDensityMapHandler.h"
#include "crownet/neighbourhood/contract/INeighborhoodTable.h"
#include <fstream>
#include "crownet/dcd/regularGrid/RegularDcdMap.h"

namespace crownet {

class NeighborhoodEventWriter : public NeighborhoodEntryListner, public BaseFileWriter
{
public:
    NeighborhoodEventWriter(std::string filePath="", std::string sep = ";", long bufferSize=8192);

    virtual ~NeighborhoodEventWriter() = default;

    virtual void onInit() override;

    virtual void neighborhoodEntryPreChanged(INeighborhoodTable* table, BeaconReceptionInfo* oldInfo) override;
    virtual void neighborhoodEntryPostChanged(INeighborhoodTable* table, BeaconReceptionInfo* info) override;

    virtual void neighborhoodEntryLeaveCell(INeighborhoodTable* table, BeaconReceptionInfo* info)override;
    virtual void neighborhoodEntryEnterCell(INeighborhoodTable* table, BeaconReceptionInfo* info)override;
    virtual void neighborhoodEntryStayInCell(INeighborhoodTable* table, BeaconReceptionInfo* info)override;
    virtual void neighborhoodEntryRemoved(INeighborhoodTable* table, BeaconReceptionInfo* info) override;
    virtual void neighborhoodEntryDropped(INeighborhoodTable* table, BeaconReceptionInfo* info) override;


    void setGlobalDensityMapHandler(IGlobalDensityMapHandler<RegularDcdMap>* globalMapHandler){
            this->globalMapHandler = globalMapHandler;
    }
    void writeData(INeighborhoodTable* table, BeaconReceptionInfo* info, std::string event);

private :
    IGlobalDensityMapHandler<RegularDcdMap>* globalMapHandler;

    void init();
};

}



