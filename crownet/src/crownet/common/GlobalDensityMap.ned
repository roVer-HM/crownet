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
        double cellSize @unit(m) = default(3.0m);
        // search these nodes for the ground truth setup. 
        string vectorNodeModules= default("misc"); // does not work with Traci managed vectors! Use traciModuleListener
        string nodeModules= default("");
        string traciModuleListener = default("traci.nodes"); // if missing it will be ignored.
        // if true use "traci.connected" to trigger density map setup. Otherwise use stage 13
        bool useSignalMapInit = default(true); 

		string writerType = default("csv"); // or sql
        double writeMapInterval @unit(s) = default(2.0s) ;
}



simple GlobalEntropyMap extends GlobalDensityMap like INeighborhoodTable 
{
  parameters:
    	@class(crownet::GlobalEntropyMap);
    	double entropyInterval @unit(s) = default(8.0s);
    	int entropyRngGenerator = 1; // default 0 used by all modules 
    	object entropyProvider;
}

