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

#ifndef CROWNET_QUEUEING_CROWNETPASSIVEPACKETSOURCEBASE_H_
#define CROWNET_QUEUEING_CROWNETPASSIVEPACKETSOURCEBASE_H_

#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

#include "crownet/queueing/CrownetPacketSourceBase.h"

using namespace inet;

namespace crownet{
namespace queueing{


class CrownetPassivePacketSourceBase : public CrownetPacketSourceBase,
                                       public virtual  inet::queueing::IPassivePacketSource{

protected:
    cGate *outputGate = nullptr;
    inet::queueing::IActivePacketSink *collector = nullptr;

    mutable Packet *nextPacket = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual Packet *providePacket(cGate *gate);

  public:
    // inet::queueing::IPacketProcessor
    virtual bool supportsPacketPushing(cGate *gate) const override { return false; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return outputGate == gate; }

    // inet::queueing::IPassivePacketSource
    virtual bool canPullSomePacket(cGate *gate) const override { return true; }

    virtual Packet *canPullPacket(cGate *gate) const override;

    virtual Packet *pullPacket(cGate *gate) override;
    virtual Packet *pullPacketStart(cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketEnd(cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Invalid operation"); }

};

}
}

#endif /* CROWNET_QUEUEING_CROWNETPASSIVEPACKETSOURCEBASE_H_ */
