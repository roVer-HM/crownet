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
import inet.common.packet.chunk.Chunk;
import inet.common.geometry.Geometry;
import inet.common.TagBase;

cplusplus{{
  using namespace inet;  
}}

namespace crownet;

class MapCfg extends cOwnedObject {
     bool writeDensityLog;
     string mapType;
     string mapTypeLog;
     string idStreamType;   
}


class DcDCell {
	uint16_t count;      // pedestrian count
	uint16_t variation;  //   
}

cplusplus(DcDCell){{
  public:
      DcDCell(uint16_t count, uint16_t variation):
      	count(count), variation(variation){}
}}

class LocatedDcDCell extends DcDCell {
    uint16_t idOffsetX;
    uint16_t idOffsetY;
}

cplusplus(LocatedDcDCell){{
  public:
      LocatedDcDCell(uint16_t count, uint16_t variation, uint16_t idOffsetX, uint16_t idOffsetY):
      	DcDCell(count, variation), idOffsetX(idOffsetX), idOffsetY(idOffsetY) {}
}}

class BaseMapPacket extends inet::FieldsChunk {
    uint16_t sequenceNumber;
 	uint32_t sourceId;
 	uint32_t timestamp; 
 	uint8_t  version = 0;
 	uint8_t	 reserverd = 0;
 	uint32_t refCoordLon; // X in microdegree
 	uint32_t refCoordLat; // Y in microdegree
 	uint16_t refIdOffsetX;
	uint16_t refIdOffsetY;
	
	chunkLength = B(24);
}

class DenseMapPacket extends BaseMapPacket {
    version = 0x10;    
    chunkLength = getChunkLength() + B(2);
    uint8_t cellCountX;
    uint8_t cellCountY;
    DcDCell cells[];
}

class SparseMapPacket extends BaseMapPacket {
    version = 0x20;
	LocatedDcDCell cells[];
}


