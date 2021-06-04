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


package crownet.simulations.networks;

import crownet.simulations.networks.World;
import inet.node.inet.AdhocHost;



network World_Wlan extends World
{
    parameters:
        int numStationary = default(0);
        bool hasStationary = default(numStationary != 0);

        hasLte = false;
        hasLteCoreNetwork = false;
        hasWlan = true;
        hasStoryboard = false;

    submodules:

        station[numStationary]: AdhocHost if hasStationary {
            parameters:
                @display("r=,,#707070;p=94.814995,266.385;i=device/accesspoint_s;t1=t");
        }
}
