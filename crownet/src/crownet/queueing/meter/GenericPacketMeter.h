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

#ifndef CROWNET_QUEUEING_METER_GENERICPACKETMETER_H_
#define CROWNET_QUEUEING_METER_GENERICPACKETMETER_H_

#include "crownet/crownet.h"
#include "inet/queueing/base/PacketMeterBase.h"
#include "inet/queueing/contract/IActivePacketSource.h"
#include "inet/queueing/contract/IPacketMeter.h"
#include "crownet/queueing/meter/meter_m.h"

using namespace inet::queueing;

namespace crownet {

class GenericPacketMeter : public PacketMeterBase{

protected:
    PackerMeterBase* meter;
    bool markPackets;

public:
    virtual ~GenericPacketMeter();

protected:
  virtual void initialize(int stage) override;
  virtual void meterPacket(inet::Packet *packet) override;

  virtual const char *resolveDirective(char directive) const override;

};

}

#endif /* CROWNET_QUEUEING_METER_GENERICPACKETMETER_H_ */
