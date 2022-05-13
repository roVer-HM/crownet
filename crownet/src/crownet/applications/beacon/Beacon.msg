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
import inet.common.packet.chunk.Chunk;
import inet.applications.base.ApplicationPacket;
import inet.common.geometry.Geometry;

import crownet.applications.common.info.AppInfo;

cplusplus {{
#include "crownet/applications/common/info/AppInfoReception.h"
}}




namespace crownet;



class BeaconBase extends inet::ApplicationPacket {
    
}

class BaseDynamicBeacon extends BeaconBase {
    
    chunkLength = b(16+32+32); // fixed
    
 	uint16_t sequencenumber; 	// 16bit
 	uint32_t sourceId;			// 32bit
 	uint32_t timestamp;			// 32bit
}

class DynamicBeaconPacket extends BaseDynamicBeacon
{
	chunkLength = chunkLength + b(2*64 + 3*12 + 16 + 12 + 12 + 4);
	
    inet::Coord pos;		//2*64bit
    inet::Coord epsilon;	//3*12bit (A.119)
    uint16_t numberOfNeighbours; // 16 bit
    uint16_t speed; // 12bit
    uint16_t heading; // 12bit
    // alignment 4bit
}

class BeaconReceptionInfo extends AppInfoReception {
    @customize(true);
    inet::Coord positionCurrent;
    inet::Coord epsilonCurrent;   
    uint16_t numberOfNeighboursCurrent; 
    double beaconValueCurrent = 1.0;
    
    inet::Coord positionPrio;
    inet::Coord epsilonPrio;   
    uint16_t numberOfNeighboursPrio; 
    double beaconValuePrio = 1.0;
};

