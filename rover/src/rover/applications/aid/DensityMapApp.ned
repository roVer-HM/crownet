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

package rover.applications.aid;
import rover.applications.common.AidBaseApp;
import inet.applications.contract.IApp;

moduleinterface IDensityMapApp extends IApp {
    parameters:
    	
}

simple ArteryDensityMapApp  extends AidBaseApp like IDensityMapApp  {
    
    parameters:
        @class(rover::ArteryDensityMapApp);
        @signal[RegisterDensityMap];
        @signal[RemoveDensityMap];
        string middelwareModule = default("^.^.middleware"); //node internal
        string identiyRegistryModule = default("identiyRegistry");
        string coordConverterModule;
        double gridSize = default(3.0) @units(m);
        packetName = default("DcDMap");
        
        destPort = 30001;
        localPort = 30001;
        appMainInterval = default(1.0s);
        
        bool writeDensityLog = default(true);
        string mapType = default("ymf"); // select map algorithm to calculate cell values
        string mapTypeLog = default("all"); // if writeDensityLog is true log this map algorithm. If "all" just 
        									// dump raw data currently present. 
        
            
}



//simple SimpleNeighbourhood like INeighbourhood {
//    
//}