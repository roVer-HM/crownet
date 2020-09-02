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
import rover.applications.common.AidBaseApp;
import inet.applications.contract.IApp;

moduleinterface INeighbourhood extends IApp {
    parameters:
    	
}

simple ArteryNeighbourhood  extends AidBaseApp like INeighbourhood  {
    
    parameters:
        @class(rover::ArteryNeighbourhood);
        string middelwareModule = default("^.^.middleware"); //node internal
        string traciCoreModule = default("traci.core");
        string coordConverterModule;
        double gridSize = default(3.0) @units(m);
        
        destPort = 30001;
        localPort = 30001;
        appMainInterval = default(1.0s);
            
}


//simple SimpleNeighbourhood like INeighbourhood {
//    
//}