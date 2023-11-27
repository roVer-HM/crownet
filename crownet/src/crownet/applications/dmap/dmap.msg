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
  #include <sstream>
  #include <string>
  using namespace inet;  
}}

namespace crownet;

class MapCfg extends cOwnedObject {
     bool writeDensityLog;
     string mapType;    
     string mapTypeLog;
     simtime_t cellAgeTTL @editable;
     string idStreamType;
     bool appendRessourceSharingDomainId = false;
     
}

cplusplus(MapCfg){{
  public:
  	virtual std::string str() const override {
 		std::stringstream s;
 		s << "mapType:" << mapType << " cellAgeTTL: " << cellAgeTTL;
 		return s.str();
  	}
}}


class MapCfgYmf extends MapCfg {
    mapType = "ymf";
}

class MapCfgYmfPlusDist extends MapCfgYmf {
 	mapType = "ymfPlusDist";
 	double alpha;   
}

class MapCfgYmfPlusDistStep extends MapCfgYmfPlusDist {
 	mapType = "ymfPlusDistStep";
 	double stepDist;
}
cplusplus(MapCfgYmfPlusDistStep){{
  public:
  	virtual std::string str() const override {
 		std::stringstream s;
 		s << MapCfgYmfPlusDist::str();
 		s << "stepDist:" << stepDist;
 		return s.str();
  	}
}}


class DcDCell {
	uint16_t count;         // 2B pedestrian count
	uint16_t deltaCreation; // 2B delte to packet creation time in 1/10 seconds
	double count_dummy;		// No size, contains count value for simulation processing only
}

cplusplus(DcDCell){{
  public:
      DcDCell(uint16_t count): count(count){}
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
    double sourceEntryDist = 0.; // 2B in meter
    
}

cplusplus(LocatedDcDCell){{
  public:
      LocatedDcDCell(uint16_t count, uint16_t idOffsetX, uint16_t idOffsetY):
      	DcDCell(count), idOffsetX(idOffsetX), idOffsetY(idOffsetY) {}
}}


class LocatedDcDCellWithSharingDomainId extends LocatedDcDCell {
 	uint16_t sharingDominId = 0;   // 2B
}
cplusplus(LocatedDcDCellWithSharingDomainId){{
  public:
      LocatedDcDCellWithSharingDomainId(uint16_t count, uint16_t idOffsetX, uint16_t idOffsetY, int rsdId):
      	LocatedDcDCell(count, idOffsetX, idOffsetY), sharingDominId(rsdId)  {}
}}


class EntropyMap extends inet::TagBase
{}


enum MapType {
    DENSE = 0x10;   
 	SPARSE = 0x20;
 	SPARSE_RSD = 0x21;
}

class MapHeader extends inet::FieldsChunk {
    uint16_t sequenceNumber;   // 2B
 	uint32_t sourceId; 			// 4B
 	uint32_t timestamp;  // 4B in ms
 	MapType  version = MapType::SPARSE; // DenseCells 0x10 | SparseCells 0x20

 	uint16_t cellIdOffsetX; // 2B
	uint16_t cellIdOffsetY; // 2B
	inet::Coord sourcePosition;  //2*8B (A.???/A.???)
	    
    chunkLength = B(2+4+4+2+2+2*8); // = 30B
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
    cellSize = B(10); // count 2B + deltaCreation 2B + idOffsetX/Y 2*2B + sourceEntryDist 2B
	LocatedDcDCell cells[];
}

class SparseMapPacketWithSharingDomainId extends MapPacketBase {
    cellSize = B(12); // count 2B + deltaCreation 2B + idOffsetX/Y 2*2B + sourceEntryDist 2B + enbID
	LocatedDcDCellWithSharingDomainId cells[];
}

