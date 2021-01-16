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

import inet.node.inet.INetworkNode;
import inet.mobility.contract.IMobility;
import inet.networklayer.common.InterfaceTable;
import inet.linklayer.contract.IWirelessInterface;

import artery.application.VehicleMiddleware;
import artery.networking.Vanetza;
import artery.nic.IRadioDriver;
import artery.inet.RadioDriver;

module ArteryPerson like IMobileUe
{
    parameters:
        @networkNode;
        @display("i=misc/smartphone;is=vs");
        @labels(node,ethernet-node,wireless-node);
        @statistic[posX](source="xCoord(mobilityPos(mobilityStateChanged))"; record=vector?);
        @statistic[posY](source="yCoord(mobilityPos(mobilityStateChanged))"; record=vector?);

        bool withAntennaMobility = default(false);
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        string mobilityType = default("VaderePersonTracedMobility");
        int numRadios = default(1);
        vanetza.position.typename = "RoVerPositionProvider";

    gates:
        input radioIn[numRadios] @directIn;
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);

    submodules:
        mobility: <mobilityType> like IMobility {
            parameters:
                @display("p=50,200");
        }

        interfaceTable: InterfaceTable {
            parameters:
                @display("p=50,300");
        }

        wlan[numRadios]: <default("VanetNic")> like IWirelessInterface {
            parameters:
                @display("p=250,400,row,100;q=queue");
                radio.antenna.mobilityModule = absPath(withAntennaMobility ? "^.antennaMobility" : "^.mobility");
        }

        radioDriver[numRadios]: RadioDriver {
            parameters:
                @display("p=250,300,row,100");
                macModule = ".wlan[" + string(index) + "].mac";
        }

        vanetza: Vanetza {
            parameters:
                @display("p=250,200");
                *.middlewareModule = absPath("^.middleware");
                *.mobilityModule = absPath("^.mobility");
                *.radioDriverModule = absPath("^.radioDriver[0]");
                runtime.datetime = middleware.datetime;
        }

        middleware: VehicleMiddleware {
            parameters:
                @display("p=250,100");
                routerModule = ".vanetza.router";
                mobilityModule = ".mobility";
        }

    connections:
        for i=0..sizeof(radioIn)-1 {
            radioIn[i] --> { @display("m=s"); } --> wlan[i].radioIn;

            wlan[i].upperLayerOut --> radioDriver[i].lowerLayerIn;
            wlan[i].upperLayerIn <-- radioDriver[i].lowerLayerOut;
        }

        radioDriver[0].upperLayer <--> vanetza.radioDriverData;
        radioDriver[0].properties --> vanetza.radioDriverProperties;

}
