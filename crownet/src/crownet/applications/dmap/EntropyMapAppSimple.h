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
#include "inet/mobility/contract/IMobility.h"
#include "crownet/applications/dmap/BaseDensityMapApp.h"

using namespace omnetpp;
using namespace inet;

namespace crownet {

class EntropyMapAppSimple : public BaseDensityMapApp

{
public:
    EntropyMapAppSimple(){};
    virtual ~EntropyMapAppSimple() = default;

protected:
 // cSimpleModule
 virtual void initialize(int stage) override;

 // Packet creation
 virtual void applyContentTags(Ptr<Chunk> content) override;


 // IDensityMapHandler
 virtual void updateLocalMap() override;


private:
  // application
 EntropyNeigborhoodTableClient *entropyClient  = nullptr;
 simtime_t lastEntropyUpdate = -1.0;
 simtime_t lastEntropySourceUpdate = -1.0;

};

} // namespace crownet
