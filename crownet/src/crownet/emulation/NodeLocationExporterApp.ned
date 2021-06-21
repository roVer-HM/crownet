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

package crownet.emulation;

import inet.applications.contract.IApp;
import inet.emulation.transportlayer.udp.ExtLowerUdp;

module NodeLocationExporterApp like IApp
{
    parameters:
        int port;
        string address;
        string interfaceTableModule;
        int xOffset = default(0);
        int yOffset = default(0);
        double interval;
        
        @display("bgb=225,330");
        
    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);

    submodules:
        extLowerUdp: ExtLowerUdp {
            @display("p=101,249");
        }

        nodeLocationExporter: NodeLocationExporter {
			port = port;
			address = address;
			interfaceTableModule = interfaceTableModule;
			xOffset = xOffset;
			yOffset = yOffset;
			interval = interval;
            @display("p=101,84");
        }

    connections allowunconnected:
        nodeLocationExporter.socketInExternal <-- extLowerUdp.appOut;
        nodeLocationExporter.socketOutExternal --> extLowerUdp.appIn;
}
