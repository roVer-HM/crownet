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

#include "GenericPacketMeter.h"

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/common/RateTag_m.h"


namespace crownet {

Define_Module(GenericPacketMeter);


GenericPacketMeter::~GenericPacketMeter(){
    delete meter;
}

void GenericPacketMeter::initialize(int stage)
{
    PacketMeterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL){
        meter = (PackerMeterBase*)(par("meter").objectValue()->dup());
        take(meter);
        markPackets = par("markPackets").boolValue();
    }
}

void GenericPacketMeter::meterPacket(Packet *packet)
{
    meter->meterPacket(packet);
    if (markPackets){
        auto rateTag = packet->addTagIfAbsent<RateTag>();
        rateTag->setDatarate(meter->getStats().getDataRate());
        rateTag->setPacketrate(meter->getStats().getPacketRate());
    }
}

const char *GenericPacketMeter::resolveDirective(char directive) const {

    static std::string result;
    switch (directive) {
        case 'p':
            result = std::to_string(meter->getStats().getPacketCount());
            break;
        case 'l':
            result = meter->getStats().getTotalData().str();
            break;
        case 'r':
            result = std::to_string(meter->getStats().getPacketRate());
            break;
        case 'R':
            result = meter->getStats().getDataRate().str();
            break;
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
    return result.c_str();

}


}
