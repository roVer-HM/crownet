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

namespace crownet;


cplusplus {{
using namespace inet;
}}


class AppInfoBase  extends cOwnedObject {
    uint32_t nodeId;
};


class AppInfoLocal extends AppInfoBase {    
    @customize(true);
    simtime_t walltimeStart @editable; // editable needed for ned object paramters. Fix in 6-pre13
	int clockRate = 1000; // timestamp in ms (assume all are syncronized.)
    uint32_t baseTimestamp;
    uint16_t sequencenumber;
    uint16_t sequencecycle;
};



enum PktType
{
    UNDEF    = 0;
    DATA_PKT = 1;
    CTRL_PKT = 2;
};

class AppInfoReception extends AppInfoBase {
 	@customize(true);

 	uint16_t initialSequencenumber;
    uint16_t maxSequencenumber;
    uint32_t sequencecycle;
	uint32_t packetsReceivedCount;
	// (max_sequencenumber - initial_sequencenumber) - packets_received_count
	// todo include sequencecycle in packet_loss_count? look at RTCP protocol!!
	int packetsLossCount; 
	int clockRate = 1000; // timestamp in ms (assume all are syncronized.)


	simtime_t jitter;
	simtime_t receivedTimePrio;
	uint32_t  sentTimePrio; 
	uint16_t  maxSequencenumberPrio;
	uint16_t  packetsReceivedCountPrio;
};


