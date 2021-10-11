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

package crownet.applications.dmap;
import crownet.applications.common.BaseApp;
import crownet.applications.common.IBaseApp;

moduleinterface IDensityMapApp extends IBaseApp {
    parameters:

}

simple BaseDensityMapApp  extends BaseApp like IDensityMapApp {
	parameters:
	    @display("i=block/users"); 
	    @class(crownet::ArteryDensityMapBase);
	    @signal[InitDensityMap];
        @signal[RegisterDensityMap];
        @signal[RemoveDensityMap];
        
        // Aid app override defaults
        packetName = default("DcDMap");
        allEmptyDestAddress = false;
        
        // App logic
        string coordConverterModule;
        double cellSize = default(3.0) @units(m);
        // update interval of local maps with neighborhood table 
        // This interval is independent of the sent interval of the application in 
        // its basic configuration. 
        double localMapUpdateInterval @unit(s) = default(1.0s); 
        
        object mapCfg = default(crownet::MapCfg{
            writeDensityLog: true,
            mapType: "ymf",
            mapTypeLog: "all",
            idStreamType: "insertionOrder"
        });
        bool writeDensityLog = default(true);
}

simple ArteryDensityMapApp  extends BaseDensityMapApp  {

    parameters:
        @class(crownet::ArteryDensityMapApp);
        string middelwareModule = default("^.^.middleware"); //node internal
        string identiyRegistryModule = default("identiyRegistry");
        string routerModule = default("^.^.vanetza.router");
        
}

simple DensityMapAppSimple  extends BaseDensityMapApp  {

    parameters:
        @class(crownet::DensityMapAppSimple);
         string neighborhoodTableMobdule = default("nTable");
}

