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

#ifndef CROWNET_QUEUEING_CROWNETACTIVEPACKETSOURCEBASE_H_
#define CROWNET_QUEUEING_CROWNETACTIVEPACKETSOURCEBASE_H_


#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IActivePacketSource.h"

#include "crownet/queueing/CrownetPacketSourceBase.h"
#include "crownet/queueing/ICrownetActivePacketSource.h"


using namespace inet;

namespace crownet{
namespace queueing{


class CrownetActivePacketSourceBase : public CrownetPacketSourceBase,
                                      public virtual  ICrownetActivePacketSource
{

protected:
  cGate *outputGate = nullptr;
  inet::queueing::IPassivePacketSink *consumer = nullptr;

protected:
  virtual void initialize(int stage) override;

public:
  virtual inet::queueing::IPassivePacketSink *getConsumer(cGate *gate) override { return consumer; }

  // ICrownetActivePacketSource
  virtual void producePacket() override;
  virtual void producePackets(int number) override;
  // produce number of packet(s) without exceeding maxData
  virtual void producePackets(inet::b maxData) override { throw cRuntimeError("Not Implemented here");};


  virtual bool supportsPacketPushing(cGate *gate) const override { return outputGate == gate; }
  virtual bool supportsPacketPulling(cGate *gate) const override { return false; }

  virtual void handleCanPushPacketChanged(cGate *gate) override;
  virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;

};

}
}

#endif /* CROWNET_QUEUEING_CROWNETACTIVEPACKETSOURCEBASE_H_ */
