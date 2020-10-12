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

package rover.common.positionMap;

//
// TODO auto-generated type
//
simple GlobalDensityMap
{
    parameters:
        @class(rover::GlobalDensityMap);
        string traciNodeManager = default("traci.nodes");
        string middelwareModule = default(".middleware"); // relative to moving object module
        string coordConverterModule = default("coordConverter");
        double gridSize = default(3.0) @units(m);
        
        double updateInterval = default(2.0s) @unit(s);
        
}
