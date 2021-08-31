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



class BeaconPacketSimple extends inet::ApplicationPacket
{
    int nodeId;
    simtime_t time;
    inet::Coord pos;
    inet::Coord epsilon;
};

class DynamicBeaconPacket extends inet::ApplicationPacket
{
    chunkLength = B(4*64 + 32 + 16);
    
    inet::Coord pos;
    inet::Coord epsilon;
    uint32_t posTimestamp;
    uint16_t numberOfNeighbours;    
}

class DynamicBeaconHeader extends inet::FieldsChunk {
    
    chunkLength = B(16+32+32); // fixed
    
 	uint16_t sequencenumber;
 	uint32_t sourceId;
 	uint32_t timestamp;
}

class BeaconReceptionInfo extends AppInfoReception {
    @customize(true);
    inet::Coord pos;
    inet::Coord epsilon;   
    uint16_t numberOfNeighbours; 
};
