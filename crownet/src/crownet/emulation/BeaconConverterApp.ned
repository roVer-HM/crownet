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


module BeaconConverterApp like IApp
{
    parameters:
        int localPort;
        int externalPort = default(1510);
        string interfaceTableModule;
        string externalAddress;
        int offsetNorthing;
        int offsetEasting;

    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);

    submodules:
        densityMessageHandler: DensityMessageHandler {
            localPort = localPort;
            localAddress = "0.0.0.0";
            externalPort = externalPort;
            externalAddress = externalAddress;
            offsetNorthing = offsetNorthing;
            offsetEasting = offsetEasting;
           	interfaceTableModule = interfaceTableModule;
            @display("p=100,200");
            	
        }

        extLowerUdp: ExtLowerUdp {
            @display("p=100,100");
        }
    connections:
        densityMessageHandler.socketIn <-- socketIn;
        densityMessageHandler.socketOut --> socketOut;
        densityMessageHandler.socketInExternal <-- extLowerUdp.appOut;
        densityMessageHandler.socketOutExternal --> extLowerUdp.appIn;
}
