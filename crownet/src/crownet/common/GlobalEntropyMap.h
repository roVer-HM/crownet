//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#pragma once

#include "crownet/neighbourhood/EntropyNeigborhoodTableClient.h"
#include "crownet/common/GlobalDensityMap.h"
#include "crownet/common/entropy/EntropyProvider.h"

using namespace omnetpp;
using namespace inet;

namespace crownet {


class GlobalEntropyMap : public GlobalDensityMap ,
                         public IBaseNeighborhoodTable {

public:

    virtual ~GlobalEntropyMap();

    // cSimpleModule
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    // ITraciNodeVisitor
    virtual void visitNode(const std::string& traciNodeId, omnetpp::cModule* mod) override;

    virtual std::string getMapName() const override;


    // IBaseNeighborhoodTable
    virtual NeighborhoodTableIter_t iter() override;
    virtual NeighborhoodTableIter_t iter(NeighborhoodTablePred_t predicate) override;
    virtual NeighborhoodTableValue_t getValue(const GridCellID& cellId);
    virtual NeighborhoodTableValue_t getValue(const int sourceId);
    virtual NeighborhoodTableValue_t getValue(const inet::Coord& pos);

protected:
 virtual void handleMessage(cMessage *msg) override;
 virtual void updateMaps() override;
 virtual void updateEntropy();


protected:
 NeighborhoodTable_t _table;
 cMessage *entropyTimer = nullptr;
 simtime_t entropyTimerInterval;
 EntropyProvider *entropyProvider = nullptr;

};





}
