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

//
// TODO generated message class
//

import inet.common.INETDefs;
import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.common.Units;
import inet.common.TagBase;

cplusplus {{
#include "inet/common/packet/Packet.h"
using namespace inet;
}}

namespace crownet;


enum PktType
{
    UNDEF    = 0;
    DATA_PKT = 1;
    CTRL_PKT = 2;
};

class PacketInfo  extends cOwnedObject {
	uint32_t sourceId;
	//
	uint32_t  creationTimeStamp;
	simtime_t creationTime @editable;
	//
	simtime_t receivedTime @editable;
    uint32_t sequenceNumber;
    bool outOfOrder=false;
}
cplusplus(PacketInfo){{
  public:
  	const simtime_t delay()const {return receivedTime-creationTime;}
}}


class AppInfoBase  extends cOwnedObject {
    uint32_t nodeId = 0;
};


class AppInfoLocal extends AppInfoBase {    
    @customize(true);
    simtime_t walltimeStart @editable; // editable needed for ned object paramters. Fix in 6-pre13
	int clockRate = 1000; // timestamp in ms (assume all are syncronized.)
    uint32_t baseTimestamp = 0;
    uint16_t sequencenumber = 0;
    uint16_t sequencecycle = 0;
};

class AppRxInfoBase extends AppInfoBase {
    
}
cplusplus(AppRxInfoBase){{
  public:
  	virtual void processInbound(const Packet *packetIn, const int rcvStationId,
            const simtime_t arrivalTime) {/*do nothing*/};
    virtual void computeMetrics(const Packet *packetIn) {/*do nothing*/};
}}


class AppRxInfo extends AppRxInfoBase{
    @customize(true);
    // Application level statistics 
    nodeId = 0; //
    // packet stream metrics
    uint32_t packetsReceivedCount = 0;  
    uint32_t packetsOctetCount = 0;
	simtime_t jitter = 0.0;  //exp smoothing with gain=1/16
	inet::b avg_packet_size = inet::b(0); //exp smoothing with gain=1/16
	// average computation based on RTPC (RFC 3350 page 31, 40)
	double ema_smoothing_jitter = 1/16;
	double ema_smoothing_packet_size = 1/16;
	
	// packet info for generic network metrics. 
	// May contain invlaid data packets such as old or out of order data. 
	PacketInfo *prioPkt @owned;
	PacketInfo *currentPkt @owned;
	simtime_t lastPktSwap @editable;
}


