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

#ifndef CROWNET_APPLICATIONS_COMMON_SCHEDULER_EVENTTRIGGERDSCHEDULERSIMPLE_H_
#define CROWNET_APPLICATIONS_COMMON_SCHEDULER_EVENTTRIGGERDSCHEDULERSIMPLE_H_


#include "crownet/applications/common/scheduler/AppSchedulerBase.h"

using namespace inet;

namespace crownet {

class EventTriggerdSchedulerSimple:  public AppSchedulerBase  {
protected:

    virtual void scheduleApp(cMessage *message) override {throw cRuntimeError("Self message Trigger not supported");}
    virtual void scheduleEvent(cMessage *message) override;
    virtual void schedulePacket(Packet *packet) override;

};


}


#endif /* CROWNET_APPLICATIONS_COMMON_SCHEDULER_EVENTTRIGGERDSCHEDULERSIMPLE_H_ */
