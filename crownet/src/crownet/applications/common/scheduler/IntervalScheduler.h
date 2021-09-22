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

#ifndef CROWNET_APPLICATIONS_COMMON_SCHEDULER_INTERVALSCHEDULER_H_
#define CROWNET_APPLICATIONS_COMMON_SCHEDULER_INTERVALSCHEDULER_H_

#include "inet/common/StringFormat.h"

#include "crownet/applications/common/scheduler/AppSchedulerBase.h"

using namespace inet;



namespace crownet {

class IntervalScheduler : public AppSchedulerBase {

protected:
   cPar *generationIntervalParameter = nullptr;
   cPar *numberPackets = nullptr;
   cPar *amoutOfData = nullptr;


   ClockEvent *generationTimer = nullptr;

   int maxNumberPackets = -1;
   int sentPackets = 0;
   B maxData = B(-1);
   B sentData = B(0);

   bool stop = false;


 protected:
   virtual void initialize(int stage) override;
   virtual void handleMessage(cMessage *message) override;

   virtual void scheduleGenerationTimer();
   virtual void scheduleApp(cMessage *message) override;
   virtual void scheduleEvent(cMessage *message) override {throw cRuntimeError("Event Trigger not supported");}
   virtual void schedulePacket(Packet *packet) override {throw cRuntimeError("Event Trigger not supported");}

 public:
   virtual ~IntervalScheduler() { cancelAndDeleteClockEvent(generationTimer); }

};


}

#endif /* CROWNET_APPLICATIONS_COMMON_SCHEDULER_INTERVALSCHEDULER_H_ */
