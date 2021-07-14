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
import crownet.emulation.locationExporter.NodeLocationExporter;
import crownet.emulation.outbound.OutboundEmulation;



module BasicEmulationApp like IApp
{
    parameters:
        int internalPort;

        int externalPort = default(1510);

        string externalAddress;
        string interfaceTableModule;

        int offsetNorthing;
        int offsetEasting;
        
        double locationExportInterval;
        

        @display("bgb=418,236");
    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);

    submodules:
        outboundEmulation: OutboundEmulation {
            localPort = internalPort;
            localAddress = "0.0.0.0";
            externalPort = externalPort;
            externalAddress = externalAddress;
            offsetNorthing = offsetNorthing;
            offsetEasting = offsetEasting;
            interfaceTableModule = interfaceTableModule;
            @display("p=114,72");

        }

        locationExporter: NodeLocationExporter {
            address = externalAddress;
            port = externalPort;
            offsetNorthing = offsetNorthing;
            offsetEasting = offsetEasting;
            interfaceTableModule = interfaceTableModule;
            interval = locationExportInterval;
            @display("p=114,164");

        }

        outboundSocket: ExtLowerUdp {
            @display("p=332,72");
        }

        locationExporterSocket: ExtLowerUdp {
            @display("p=332,164");
        }
    connections:
        outboundEmulation.socketIn <-- socketIn;
        outboundEmulation.socketOut --> socketOut;
        outboundEmulation.socketInExternal <-- outboundSocket.appOut;
        outboundEmulation.socketOutExternal --> outboundSocket.appIn;
        locationExporter.socketInExternal <-- locationExporterSocket.appOut;
        locationExporter.socketOutExternal --> locationExporterSocket.appIn;
}
