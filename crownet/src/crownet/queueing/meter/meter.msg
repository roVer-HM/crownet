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

import inet.common.INETDefs;
import inet.common.Units;
import inet.common.packet.Packet;

namespace crownet;


cplusplus {{
using namespace inet;
}}

class PacketFlowStats extends cOwnedObject {
    int packetCount;
    inet::b totalData = inet::b(0);
    inet::bps dataRate = inet::bps(0);
    double packetRate;
}

cplusplus(PacketFlowStats){{
  public:
  	void incrData(b data){totalData += data;}
  	void incrPacket(int p=1){packetCount +=p;}
}}

class PackerMeterBase extends cOwnedObject {
    PacketFlowStats stats;  
    int currentNumPackets = 0; // cummulate packets with the same timestamp
    inet::b  currentTotalPacketLength = inet::b(0); // cummulate packets with the same timestamp
    simtime_t lastUpdate = -1.0 @editable; 
}

cplusplus(PackerMeterBase){{
    public:
      virtual void meterPacket(Packet *packet) {cRuntimeError("Implement");}
      virtual void incrCurrentPacketCount(int numPackets = 1) { currentNumPackets += numPackets;}
      virtual void incrCurrentPacketLength(inet::b packetLength = inet::b(1)) { currentTotalPacketLength += packetLength;}
}}

class EmaPacketMeter extends PackerMeterBase {
    @customize(true);
    double alpha;
}


class TemaPacketMeter extends PackerMeterBase {
 	@customize(true);
 	double beta;   
}