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

#include "crownet/crownet.h"
#include "crownet/applications/common/scheduler/AppSchedulerBase.h"

using namespace inet;



namespace crownet {

enum IntervalSchedulerType {
    PACKET,
    DATA,
    OTHER,
};

class IntervalScheduler : public AppSchedulerBase {
protected:

    struct Unit {
        IntervalSchedulerType type;
        int packetCount;
        b data;

        bool validPacketUnit() const {
            return type == IntervalSchedulerType::PACKET && this->packetCount > 0;
        }
        bool validDataUnit() const {
            return type == IntervalSchedulerType::DATA && this->data > b(0);
        }

        std::string str() const {
            std::stringstream s;
            s << "[" << "type: " << type << ", packetCount: " << packetCount << ", data: " << data.str() << "]";
            return s.str();
        }
    };


protected:
   cPar *generationIntervalParameter = nullptr;
   cPar *numberPackets = nullptr;
   cPar *amountOfData = nullptr;


   ClockEvent *generationTimer = nullptr;

   int maxNumberPackets = -1;
   int sentPackets = 0;
   B maxData = B(-1);
   B sentData = B(0);
   bool stopScheduling = false;
   simtime_t startOffset;

 protected:
   virtual void initialize(int stage) override;
   virtual void handleMessage(cMessage *message) override;

   virtual void scheduleGenerationTimer();
   virtual void scheduleApp(cMessage *message) override;
   virtual void scheduleEvent(cMessage *message) override {throw cRuntimeError("Event Trigger not supported");}
   virtual void schedulePacket(Packet *packet) override {throw cRuntimeError("Event Trigger not supported");}

   virtual void scheduleByPackets(const Unit& unit);
   virtual void scheduleByData(const Unit& unit);

   virtual Unit getScheduledUnit();

 public:
   virtual ~IntervalScheduler() { cancelAndDeleteClockEvent(generationTimer); }


   const bool hasPacketMaximum() const {return maxNumberPackets > 0;}
   const bool packetMaximumReached(const int& scheduledPacketCount) const;

   const bool hasDataMaximum() const { return maxData > b(0);}
   const bool dataMaximumReached(const b& scheduledData) const;

   static simsignal_t scheduledData_s;
   static simsignal_t consumedData_s;

};


}

#endif /* CROWNET_APPLICATIONS_COMMON_SCHEDULER_INTERVALSCHEDULER_H_ */
