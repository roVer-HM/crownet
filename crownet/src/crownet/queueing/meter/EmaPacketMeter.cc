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

#include "EmaPacketMeter.h"

namespace crownet {

Register_Class(EmaPacketMeter);

void EmaPacketMeter::meterPacket(Packet *packet){

    auto now = simTime();
    if (now != lastUpdate) {
        auto elapsedTime = (now - lastUpdate).dbl();
        auto packetrateChange = (currentNumPackets / elapsedTime) - stats.getPacketRate();
        stats.setPacketRate(stats.getPacketRate() + packetrateChange * alpha);

        auto datarateChange = (currentTotalPacketLength / s(elapsedTime)) - stats.getDataRate();
        stats.setDataRate(stats.getDataRate() + datarateChange * alpha);

        currentNumPackets = 0;
        currentTotalPacketLength = b(0);
        lastUpdate = now;
    }
    currentNumPackets++;
    currentTotalPacketLength += packet->getTotalLength();
    stats.incrData(packet->getTotalLength());
    stats.incrPacket();
}

}

