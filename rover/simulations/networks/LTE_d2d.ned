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

package rover.simulations.networks;

import inet.node.inet.StandardHost;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth10G;
import inet.node.ethernet.Eth10M;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import lte.world.radio.ChannelControl;
import lte.corenetwork.binder.LteBinder;
import lte.corenetwork.nodes.Ue;
import lte.corenetwork.nodes.eNodeB;
import lte.world.radio.LteChannelControl;
import lte.epc.PgwStandardSimplified;
import rover.nodes.MobileUe;
import org.car2x.veins.modules.mobility.traci.ITraCIManager;
import org.car2x.veins.subprojects.veins_inet.mapping.NodeMappingDistribution;

network LTE_d2d
{
    parameters:
        int numUeCell = default(0);
        int numUeD2DReceiver = default(0);
        int numUeD2DSender = default(0);
        int numMappings = default(0);
        @figure[title](type=label; pos=0,-1; anchor=sw; color=darkblue);
		@figure[rcvdPkText](type=indicatorText; pos=380,20; anchor=w; font=,18; textFormat="packets received: %g"; initialValue=0);

    submodules:

        channelControl: LteChannelControl {
            @display("p=50,25;is=s");
        }
        routingRecorder: RoutingTableRecorder {
            @display("p=50,75;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,125");
            config = xmldoc("demo.xml");
        }
        binder: LteBinder {
            @display("p=50,175;is=s");
        }
        server: StandardHost {
            @display("p=212,118;is=n;i=device/server");
        }
        router: Router {
            @display("p=321,136;i=device/smallrouter");
        }
        pgw: PgwStandardSimplified {
            nodeType = "PGW";
            @display("p=519,175;is=l");
        }
        eNB: eNodeB {
            @display("p=391,259;is=vl");
        }
        ueCell[numUeCell]: Ue {
            @display("p=292,367;is=vs");
        }

        ueD2DReceiver[numUeD2DReceiver]: MobileUe {
            @display("p=477,367;is=vs");
            ipv4.configurator.typename = "HostAutoConfigurator";
            ipv4.configurator.interfaces = "wlan";
        }

        ueD2DSender[numUeD2DSender]: MobileUe {
            @display("p=477,367;is=vs");
            ipv4.configurator.typename = "HostAutoConfigurator";
            ipv4.configurator.interfaces = "wlan";
        }

        mappingDistribution[numMappings]: NodeMappingDistribution {

            @display("p=520,38");
        }
        manager: <default(TraCIScenarioManagerVadere)> like ITraCIManager if typename != "" {
            @display("p=300,38");
        }

    connections:
        server.pppg++ <--> Eth10G <--> router.pppg++;
        router.pppg++ <--> Eth10G <--> pgw.filterGate;
        pgw.pppg++ <--> Eth10G <--> eNB.ppp;
}
