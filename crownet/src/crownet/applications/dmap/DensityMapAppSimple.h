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

#include "inet/mobility/contract/IMobility.h"
#include "crownet/applications/dmap/BaseDensityMapApp.h"
#include "crownet/common/NeighborhoodTable.h"

using namespace omnetpp;
using namespace inet;

namespace crownet {

class DensityMapAppSimple : public BaseDensityMapApp {
public:
    DensityMapAppSimple(){};
    virtual ~DensityMapAppSimple()=default;

protected:
 // cSimpleModule
 virtual void initialize(int stage) override;

 // IDensityMapHandler
 virtual void updateLocalMap() override;

private:
  // application
 inet::IMobility *mobility = nullptr;
 NeighborhoodTable *nTable  = nullptr;

};

} // namespace crownet
