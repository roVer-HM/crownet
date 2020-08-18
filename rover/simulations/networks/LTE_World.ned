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

import rover.simulations.networks.LTE_d2d;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
import inet.environment.common.PhysicalEnvironment;
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
import rover.nodes.IMobileUe;
import rover.nodes.UnequippedHost;

import artery.storyboard.Storyboard;
import traci.Manager;


network LTE_World
{
    parameters:
        int numUeCell = default(0);
        int numUeD2D_a = default(0);
        int numUeD2D_b = default(0);
        int numMappings = default(0);
        string ueType = default("MobileUe");
        @figure[title](type=label; pos=0,-1; anchor=sw; color=darkblue);
        @display("bgb=899,473");

        bool withStoryboard = default(false);
        bool withCoreNetwork = default(true);
        
        traci.nodes.typename = default("VadereNodeManager");
        traci.subscriptions.typename = default("VadereSubscriptionManager");
        traci.core.typename = default("VadereCore");
        traci.launcher.typename = default("VadereLauchner");
        traci.nodes.objectSinkModule = ".mobility";
        traci.mapper.objectType = default("rover.artery.PedestrianFull");
        traci.nodes.vehicleSinkModule = default(".mobility");
        storyboard.middlewareModule = default(".middleware");

    submodules:
        visualization: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            @display("p=518,32;is=s");
        }

        physicalEnvironment: PhysicalEnvironment {
            @display("p=413,32;is=s");
        }
        channelControl: LteChannelControl {
            @display("p=412,110;is=s");
        }
        routingRecorder: RoutingTableRecorder {
            @display("p=426,78;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=524,173;is=s");
            config = xmldoc("ip4Config.xml");
        }
        binder: LteBinder {
            @display("p=403,164;is=s");
        }
        server: StandardHost if withCoreNetwork {
            @display("p=315,150;is=s;i=device/server");
        }
        router: Router if withCoreNetwork {
            @display("p=315,110;is=s;i=device/smallrouter");
        }
        pgw: PgwStandardSimplified if withCoreNetwork{
            nodeType = "PGW";
            @display("p=314,43;is=l");
        }
        eNB: eNodeB {
            @display("p=37,51;is=vl");
        }

        traci: Manager {
            parameters:
                @display("p=584,32");
        }

        storyboard: Storyboard if withStoryboard {
            parameters:
                @display("p=524,101");
        }

    connections:
        server.pppg++ <--> Eth10G <--> router.pppg++;
        router.pppg++ <--> Eth10G <--> pgw.filterGate;
        pgw.pppg++ <--> Eth10G <--> eNB.ppp;
}
