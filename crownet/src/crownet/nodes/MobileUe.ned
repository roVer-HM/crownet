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

package crownet.nodes;

import crownet.nodes.IMobileUe;
import simu5g.nodes.Ue;
import inet.networklayer.configurator.ipv4.HostAutoConfigurator;

// see lte.corenetwork.nodes.cars;
module MobileUe extends Ue like IMobileUe{

	parameters:
	    @display("i=misc/smartphone;is=vs");

	    mobility.typename = default("VaderePersonTracedMobility");

	    // Network Layer specs
        *.routingTableModule = default(absPath(".ipv4.routingTable"));

	submodules:
	    configurator: HostAutoConfigurator {
            @display("p=126.4,413.96");
        }

}

