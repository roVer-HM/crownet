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



enum PktType
{
    UNDEF    = 0;
    DATA_PKT = 1;
    CTRL_PKT = 2;
};

class AppInfoReception extends AppInfoBase {
 	@customize(true);

 	uint16_t initialSequencenumber= 0;
    uint16_t maxSequencenumber= 0;
    uint32_t sequencecycle= 0;
	uint32_t packetsReceivedCount= 0;
	// (max_sequencenumber - initial_sequencenumber) - packets_received_count
	// todo include sequencecycle in packet_loss_count? look at RTCP protocol!!
	int packetsLossCount = 0; 
	int clockRate = 1000; // timestamp in ms (assume all are syncronized.)


	simtime_t jitter = 0.0;
	simtime_t receivedTimePrio = 0.0;
	uint32_t  sentTimePrio = 0; 
	uint16_t  maxSequencenumberPrio = 0;
	uint16_t  packetsReceivedCountPrio = 0;
};


