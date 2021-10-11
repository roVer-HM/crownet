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
        string traciNodeManager = default("traci.nodes");
        string mobilityModule = default (".mobility");
        string coordConverterModule = default("coordConverter");
        double cellSize = default(3.0) @units(m);

        double updateInterval @unit(s) = default(2.0s) ;
}



simple GlobalEntropyMap extends GlobalDensityMap like INeighborhoodTable 
{
  parameters:
    	@class(crownet::GlobalEntropyMap);
}

