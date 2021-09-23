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

#include "crownet/applications/common/BaseBroadcast.h"
#include "crownet/artery/traci/VaderePersonController.h"
using namespace inet;

namespace crownet {

class BroadcastControlApp : public BaseBroadcast {
public:
    BroadcastControlApp();
    virtual ~BroadcastControlApp();

protected:
    virtual void initialize(int stage) override;

    virtual Packet *createPacket() override;

    virtual FsmState fsmHandleSubState(cMessage *msg) override;
    virtual FsmState handlePayload(const Ptr<const ApplicationPacket> pkt) override;
    bool isControlled() const { return ctrl != nullptr; }


    std::string modelString = "";
    std::string modelData = "";
    simtime_t modelAge;

    VaderePersonController* ctrl = nullptr;
};
}

