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

#include "crownet/neighbourhood/contract/INeighborhoodTable.h"

#include "inet/mobility/contract/IMobility.h"
#include "crownet/applications/dmap/BaseDensityMapApp.h"

using namespace omnetpp;
using namespace inet;

namespace crownet {

class DensityMapAppSimple : public BaseDensityMapApp
                            , public NeighborhoodEntryListner

{
public:
    DensityMapAppSimple(){};
    virtual ~DensityMapAppSimple();

protected:
 // cSimpleModule
 virtual void initialize(int stage) override;
 virtual void finish() override;

 // IDensityMapHandler
 virtual void updateLocalMap() override;
 virtual void computeValues() override;

 //NeighborhoodEntryListner
 virtual void neighborhoodEntryPreChanged(INeighborhoodTable* table, BeaconReceptionInfo* oldInfo)override;
 virtual void neighborhoodEntryPostChanged(INeighborhoodTable* table, BeaconReceptionInfo* newInfo) override;
 virtual void neighborhoodEntryRemoved(INeighborhoodTable* table, BeaconReceptionInfo* info) override;


private:
  // application
 INeighborhoodTable *nTable  = nullptr;

};

} // namespace crownet
