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
import crownet.emulation.inbound.InboundEmulation;


module BeaconConverterApp like IApp
{
    parameters:
        int internalPort;

        int outboundPort = default(1510);
        int inboundPort = default(1510);

        string externalAddress;

        string interfaceTableModule;

        int offsetNorthing;
        int offsetEasting;
        
        int deviceId;

        @display("bgb=418,236");
    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);

    submodules:
        densityMessageHandler: DensityMessageHandler {
            localPort = internalPort;
            localAddress = "0.0.0.0";
            externalPort = outboundPort;
            externalAddress = externalAddress;
            offsetNorthing = offsetNorthing;
            offsetEasting = offsetEasting;
            interfaceTableModule = interfaceTableModule;
            @display("p=114,72");

        }

        inboundEmulation: InboundEmulation {
            externalPort = inboundPort;
            externalAddress = externalAddress;
            offsetNorthing = offsetNorthing;
            offsetEasting = offsetEasting;
            interfaceTableModule = interfaceTableModule;
            deviceId = deviceId;
            @display("p=114,164");

        }

        outboundSocket: ExtLowerUdp {
            @display("p=332,72");
        }

        inboundSocket: ExtLowerUdp {
            @display("p=332,164");
        }
    connections:
        densityMessageHandler.socketIn <-- socketIn;
        densityMessageHandler.socketOut --> socketOut;
        densityMessageHandler.socketInExternal <-- outboundSocket.appOut;
        densityMessageHandler.socketOutExternal --> outboundSocket.appIn;
        inboundEmulation.socketInExternal <-- inboundSocket.appOut;
        inboundEmulation.socketOutExternal --> inboundSocket.appIn;
}
