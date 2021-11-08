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
import inet.common.Units;

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
	uint16_t count;         // 2B pedestrian count
	uint16_t variation;     // 2B   
	uint16_t deltaCreation; // 2B delte to packet creation time in 1/10 seconds
}

cplusplus(DcDCell){{
  public:
      DcDCell(uint16_t count, uint16_t variation):
      	count(count), variation(variation){}
      void setDeltaCreation(const simtime_t& delta);
      simtime_t getCreationTime(const simtime_t& base) const;
}}

cplusplus(cc){{
  void DcDCell::setDeltaCreation(const simtime_t& delta){
      if (delta < 0){
          throw cRuntimeError("delta_t cannot be negative got %s", delta.ustr().c_str());
      }
      int64_t raw_ms = (int64_t)(delta.inUnit(SimTimeUnit::SIMTIME_MS) / 10);
      if (raw_ms > 0xFFFF){
          raw_ms = 0xFFFF; // set to undefined.
      }
    deltaCreation = (uint16_t)(raw_ms);
  }
  
  simtime_t DcDCell::getCreationTime(const simtime_t& base) const{
      return base - SimTime(deltaCreation*10, SimTimeUnit::SIMTIME_MS);
  }
}}




class LocatedDcDCell extends DcDCell {
    uint16_t idOffsetX;   // 2B
    uint16_t idOffsetY;   // 2B
}

cplusplus(LocatedDcDCell){{
  public:
      LocatedDcDCell(uint16_t count, uint16_t variation, uint16_t idOffsetX, uint16_t idOffsetY):
      	DcDCell(count, variation), idOffsetX(idOffsetX), idOffsetY(idOffsetY) {}
}}

class EntropyMap extends inet::TagBase
{}


enum MapType {
    DENSE = 0x10;   
 	SPARSE = 0x20;
}

class MapHeader extends inet::FieldsChunk {
    uint16_t sequenceNumber;
 	uint32_t sourceId; 
 	uint32_t timestamp;  // in ms
 	MapType  version = MapType::SPARSE;	 // DenseCells 0x10 | SparseCells 0x20
 	uint8_t	 reserverd = 0;
 	uint32_t refCoordLon; // X in microdegree
 	uint32_t refCoordLat; // Y in microdegree
 	uint16_t refIdOffsetX;
	uint16_t refIdOffsetY;
	
	// todo check
 	uint16_t sourceCellIdX; // 2B
	uint16_t sourceCellIdY; // 2B
    uint16_t numberOfNeighbours; // 2B
    
	chunkLength = B(24 + 6);
}

class MapPacketBase extends inet::FieldsChunk {
    chunkLength = B(0); // base length
 	inet::b cellSize;   
}

class DenseMapPacket extends MapPacketBase {   
    chunkLength = B(2); // base length
    cellSize = B(6);
    uint8_t cellCountX;
    uint8_t cellCountY;
    DcDCell cells[];
}

class SparseMapPacket extends MapPacketBase {
    cellSize = B(10);
	LocatedDcDCell cells[];
}


