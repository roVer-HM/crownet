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


package crownet.simulations.coviCom21;

import crownet.simulations.networks.LTE_d2d;
import inet.visualizer.contract.IIntegratedVisualizer;
import inet.environment.common.PhysicalEnvironment;
import inet.node.inet.StandardHost;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth10G;
import inet.node.ethernet.Eth10M;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadioMedium;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import simu5g.world.radio.ChannelControl;
import simu5g.common.binder.Binder;
import simu5g.nodes.Ue;
import simu5g.nodes.eNodeB;
import simu5g.world.radio.LteChannelControl;
import simu5g.nodes.PgwStandard;
import simu5g.common.carrierAggregation.CarrierAggregation;
import crownet.nodes.IMobileUe;
import crownet.nodes.UnequippedHost;
import crownet.common.ICoordConverter;
import crownet.common.IGlobalDensityMap;

import artery.storyboard.Storyboard;
import artery.utility.IdentityRegistry;
import traci.Manager;


network World
{
    parameters:
        @figure[title](type=label; pos=0,-1; anchor=sw; color=darkblue);
        @display("bgb=899,473");

        bool hasCellular = default(true);
        bool hasCellularCoreNetwork = default(hasCellular);
        bool hasWlan = default(false);
        bool hasStoryboard = default(false);

        traci.nodes.typename = default("VadereNodeManager");
        traci.nodes.personSinkModule = default(".mobility");
        traci.subscriptions.typename = default("VadereSubscriptionManager");
        traci.mapper.personType = default("crownet.nodes.AidPedestrian");
        traci.mapper.vehicleType = default("crownet.nodes.AidPedestrian");

        traci.core.version = default(-1);
        traci.core.selfStopping = default(false);
        traci.core.typename = default("VadereCore");

        traci.launcher.typename = default("VadereLauchner");
        traci.launcher.hostname = default("vadere");
		traci.launcher.port = default(9998);


        storyboard.middlewareModule = default(".middleware");

    submodules:

        coordConverter: <default("OsgCoordConverterVadere")> like ICoordConverter if typename != "" {
            @display("p=308.14874,207.69;i=block/table2;is=s");
        }
        visualization: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            @display("p=523.74,89.17125;is=s");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=413,32;is=s");
        }
        routingRecorder: RoutingTableRecorder {
            @display("p=411.99374,89.17125;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=523.74,198.66;is=s");
        }
        // LTE
        channelControl: LteChannelControl if hasCellular {
            @display("p=410.865,148.995;is=s");
        }
        binder: Binder if hasCellular {
            @display("p=411.99374,198.66;is=s");
        }
        carrierAggregation: CarrierAggregation {
            @display("p=8,5;is=s");
        }
        eNB: eNodeB if hasCellular {
            @display("p=37,51;is=vl");
        }
        server: StandardHost if hasCellularCoreNetwork {
            @display("p=308.14874,148.995;is=s;i=device/server");
        }
        router: Router if hasCellularCoreNetwork {
            @display("p=308.14874,110.6175;is=s;i=device/smallrouter");
        }
        pgw: PgwStandard if hasCellularCoreNetwork {
            @display("p=307.02,42.892498;is=l");
        }
        // WLAN
        radioMedium: <default("Ieee80211ScalarRadioMedium")> like IRadioMedium if hasWlan {
            parameters:
                @display("p=423.28125,267.51373;is=s");
        }
        // TraCI
        traci: Manager {
            parameters:
                @display("p=523.74,31.605");
        }
        identiyRegistry: IdentityRegistry {
            @display("p=523.74,259.6125;i=block/table2;is=s");
        }
        // Misc
        storyboard: Storyboard if hasStoryboard {
            parameters:
                @display("p=523.74,148.995");
        }
        gloablDensityMap: <default("")> like IGlobalDensityMap if typename != "" {
            @display("p=308.14874,268.6425;i=block/table2;is=s");
        }


    connections:
        server.pppg++ <--> Eth10G <--> router.pppg++ if hasCellularCoreNetwork;
        router.pppg++ <--> Eth10G <--> pgw.filterGate if hasCellularCoreNetwork;
        pgw.pppg++ <--> Eth10G <--> eNB.ppp if hasCellularCoreNetwork;
}
