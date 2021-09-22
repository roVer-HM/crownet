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

#ifndef CROWNET_APPLICATIONS_COMMON_SCHEDULER_APPSCHEDULERBASE_H_
#define CROWNET_APPLICATIONS_COMMON_SCHEDULER_APPSCHEDULERBASE_H_


#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/common/clock/ClockUserModuleMixin.h"

#include "crownet/queueing/ICrownetActivePacketSource.h"
#include "crownet/applications/common/scheduler/IAppScheduler.h"

using namespace inet;
using namespace crownet::queueing;

namespace crownet {

class AppSchedulerBase :
            public IAppScheduler,
            public ClockUserModuleMixin<inet::queueing::PacketProcessorBase> {
protected:
    cGate *outputGate = nullptr;
    cGate *schedulerIn = nullptr;
    ICrownetActivePacketSource *consumer = nullptr;

protected:
    virtual void initialize(int stage) override;
    // handle IAppScheduler without any additional logic
    virtual void handleMessage(cMessage *message) override;


public:
    virtual bool supportsPacketPushing(cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }
};

}


#endif /* CROWNET_APPLICATIONS_COMMON_SCHEDULER_APPSCHEDULERBASE_H_ */
