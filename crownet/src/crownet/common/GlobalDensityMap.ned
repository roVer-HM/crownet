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

package crownet.common;
import crownet.neighbourhood.INeighborhoodTable;

//
// TODO auto-generated type
//
moduleinterface IGlobalDensityMap {}

simple GlobalDensityMap like IGlobalDensityMap
{
    parameters:
        @class(crownet::GlobalDensityMap);
        string mobilityModule = default (".mobility");
        string coordConverterModule = default("coordConverter");
        double cellSize @unit(m) = default(5.0m);

		int mapSqlMaxCommitCount = default(200); // number of insert statements to inlcude in one sql transaction if writeType is 'sql'
		string writerType = default("csv"); // or sql
        double writeMapInterval @unit(s) = default(2.0s) ;
        
        object mapCfg = default(crownet::MapCfg{
            writeDensityLog: true,
            mapType: "global",
            mapTypeLog: "all",
            cellAgeTTL: -1.0s,
            idStreamType: "insertionOrder",
            appendRessourceSharingDomainId: false
        });
        
        // search these nodes for the ground truth setup. 
        string vectorNodeModule= default("misc"); // does not work with Traci managed vectors! Use traciModuleListener
        double deleteTime @unit(s) = default(-1.0s);
        double deletePercentage = default(0.0);
        string nodeModules= default("");
        string traciModuleListener = default("traci.nodes"); // if missing it will be ignored.
        // if true use "traci.connected" to trigger density map setup. Otherwise use stage 13
        bool useSignalMapInit = default(true); 
}




simple GlobalEntropyMap extends GlobalDensityMap like INeighborhoodTable 
{
  parameters:
    	@class(crownet::GlobalEntropyMap);
    	double entropyInterval @unit(s) = default(8.0s);
    	int entropyRngGenerator = 1; // default 0 used by all modules 
    	object entropyProvider;
    	
}

