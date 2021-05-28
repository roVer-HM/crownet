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

package crownet.applications.aid;
import crownet.applications.common.BaseApp;
import inet.applications.contract.IApp;

moduleinterface IDensityMapApp extends IApp {
    parameters:

}

simple BaseDensityMapApp  extends BaseApp like IDensityMapApp {
	parameters:
	    @class(crownet::ArteryDensityMapBase);
        @signal[RegisterDensityMap];
        @signal[RemoveDensityMap];
        
        // Aid app override defaults
        packetName = default("DcDMap");
        appMainInterval = default(1.0s);
        messageLength = default(1kB);
        

	    // Aid app fix defaults
//        destPort = 30001;
//        localPort = 30001;         
        
        // App logic
        string coordConverterModule;
        
        double gridSize = default(3.0) @units(m);
        
        bool writeDensityLog = default(true);
        string mapType = default("ymf"); // select map algorithm to calculate cell values
        string mapTypeLog = default("all"); // if writeDensityLog is true log this map algorithm. If "all" just
        									// dump raw data currently present.
}

simple ArteryDensityMapApp  extends BaseDensityMapApp  {

    parameters:
        @class(crownet::ArteryDensityMapApp);
        string middelwareModule = default("^.^.middleware"); //node internal
        string identiyRegistryModule = default("identiyRegistry");
        
}

simple DensityMapAppSimple  extends BaseDensityMapApp  {

    parameters:
        @class(crownet::DensityMapAppSimple);

         string mobilityModule;
         string neighborhoodTableMobdule;
}

//simple SimpleNeighbourhood like INeighbourhood {
//
//}
