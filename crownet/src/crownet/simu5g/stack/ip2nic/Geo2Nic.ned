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

package crownet.simu5g.stack.ip2nic;

import simu5g.stack.ip2nic.INetworkLayer2CellularNic;

simple Geo2Nic like INetworkLayer2CellularNic
{
    parameters:
        string nodeType;
        string interfaceTableModule;
        string routingTableModule;
        string interfaceName = default("cellular");
        @display("i=block/layer");
        @class(crownet::Geo2Nic);
    gates:
        // connection to network layer.
        // These gate names are recognized by the network layer (same as the ones in the INetworkInterface interface)
        input upperLayerIn;
        output upperLayerOut;

        // connection to the cellular stack
        inout stackNic @labels(IPDatagram);
}
