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

#include "TemaPacketMeter.h"
#include <math.h>

namespace crownet {

Register_Class(TemaPacketMeter);

void TemaPacketMeter::meterPacket(Packet *packet){


    auto now = simTime();
    if (lastUpdate < simtime_t::ZERO){
        // first call: assume elapsedTime (now - 0.0) = now
        auto elapsedTime = now.dbl();
        if (elapsedTime <= 0.0){
            stats.setPacketRate(1.0);
            stats.setDataRate(bps(packet->getDataLength()/ s(1.0)));
        } else {
            stats.setPacketRate(1.0 / elapsedTime);
            stats.setDataRate(bps(packet->getDataLength()/ s(elapsedTime)));
        }
        // reset count for packets that arrive at the same time
        setCurrentNumPackets(0);
        setCurrentTotalPacketLength(b(0));
        setLastUpdate(now);
    } else if (now > lastUpdate){
        auto elapsedTime = (now - lastUpdate).dbl();
        stats.incrPacket(getCurrentNumPackets());
        stats.incrData(getCurrentTotalPacketLength());

        double f = exp(-1 * getBeta() * elapsedTime);
        double ff = 1 - f;


        double pRate = f * stats.getPacketRate() + ff * 1/elapsedTime; // packet rate
        stats.setPacketRate(pRate);

        double dRate = f * stats.getDataRate().get() + ff * packet->getTotalLength().get()/elapsedTime; // data rate
        stats.setDataRate(bps(dRate));

        // reset count for packets that arrive at the same time
        setCurrentNumPackets(0);
        setCurrentTotalPacketLength(b(0));
        setLastUpdate(now);
    }
    incrCurrentPacketCount();
    incrCurrentPacketLength(packet->getTotalLength());
}

} // namespace
