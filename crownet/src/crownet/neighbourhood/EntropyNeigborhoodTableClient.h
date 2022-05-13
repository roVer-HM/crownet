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

#include <omnetpp/cobject.h>
#include "inet/common/InitStages.h"
#include "crownet/neighbourhood/contract/INeighborhoodTable.h"
#include "crownet/common/IDensityMapHandler.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"
#include "crownet/common/MobilityProviderMixin.h"


using namespace omnetpp;
using namespace inet;

namespace crownet {

class EntropyNeigborhoodTableClient : public MobilityProviderMixin<cSimpleModule>,
                                      public INeighborhoodTable {
public:
    virtual ~EntropyNeigborhoodTableClient() = default;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    virtual BeaconReceptionInfo* getOrCreateEntry(const int sourceId) override;
    virtual bool processInfo(BeaconReceptionInfo *packet) override {return true;} //todo
    virtual void checkAllTimeToLive() override {}; // do nothing as client
    virtual bool ttlReached(BeaconReceptionInfo*) override {return true;}// do nothing as client
    virtual void setOwnerId(int ownerId) override {this->ownerId = ownerId;}
    virtual const int getOwnerId() const override {return ownerId;}
    virtual const int getSize() override;


    // default to distance based iterator because this class accesses the global table.
    virtual NeighborhoodTableIter_t iter() override;
    virtual NeighborhoodTableIter_t iter(NeighborhoodTablePred_t predicate) override;


protected:
    IBaseNeighborhoodTable* globalTable;
    double dist = 0;
    int ownerId = 0;
};


}
