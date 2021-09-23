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

#include "crownet/applications/common/AppFsm.h"
#include "crownet/applications/common/BaseApp.h"


namespace crownet {
class BaseBroadcast : public BaseApp{
public:
    BaseBroadcast();
    virtual ~BaseBroadcast();

    virtual FsmState handleDataArrived(Packet *packet) override;
    virtual FsmState handlePayload(const Ptr<const ApplicationPacket> pkt);

    virtual Packet *createPacket() override;
protected:
    // cSimpleModule
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

protected:
    int initialHopCount = 1;
};
}

