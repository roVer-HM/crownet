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

package rover.aid;

moduleinterface INeighbourhood {
    parameters:
    	
}

simple ArteryNeighbourhood like INeighbourhood {
    
    parameters:
        @class(rover::ArteryNeighbourhood);
        string middelwareModule = default("^.^.middleware"); //node internal
        string traciCoreModule = default("traci.core");
        string coordConverterModule;
        double updateInterval = default(1.0s) @unit(s); // interval to query artery router for current 
        											   // state of the map.
        double sendInterval = default(1.0s) @unit(s); // interval to query artery router for current 
        											   // state of the map.
        											   
        double gridSize = default(3.0) @units(m);
    
}


//simple SimpleNeighbourhood like INeighbourhood {
//    
//}