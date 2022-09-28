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


package crownet.simulations.networks;

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
import inet.node.contract.INetworkNode;
import inet.common.geometry.common.IGeographicCoordinateSystem;
import simu5g.world.radio.ChannelControl;
import simu5g.common.binder.Binder;
import simu5g.common.carrierAggregation.CarrierAggregation;
import simu5g.nodes.Ue;
import simu5g.nodes.eNodeB;
import simu5g.nodes.NR.gNodeB;
import simu5g.world.radio.LteChannelControl;
import simu5g.nodes.PgwStandard;
import simu5g.nodes.Upf;
import crownet.nodes.IMobileUe;
import crownet.nodes.UnequippedHost;
import crownet.common.ICoordConverter;
import crownet.common.IGlobalDensityMap;
import crownet.control.IControlManager;
import crownet.common.util.IFileWriterRegister;
import crownet.mobility.BonnMotionServer;
import crownet.nodes.ApplicationLayerPedestrian;
import crownet.nodes.ApplicationLayerVehicle;

import artery.storyboard.IStoryboard;
import artery.utility.IdentityRegistry;
import traci.Manager;
import traci.TraCiManager;


network World
{
    parameters:
        @figure[title](type=label; pos=0,-1; anchor=sw; color=darkblue);
        @display("bgb=899,473");

        bool hasCellular = default(true);
        bool hasCellularCoreNetwork = default(hasCellular);
        bool hasStoryboard = default(false);
        bool hasWlan = default(false);
        bool useNR = default(false);
        int numMisc = default(0);
        int numEnb = default(1);
        int numGnb = default(1);

        traci.core.typename = default("VadereCore");
        traci.core.version = default(-1);
        traci.core.selfStopping = default(false);
        traci.launcher.typename = default("VadereLauncher");
        traci.launcher.hostname = default("vadere");
        traci.launcher.port = default(9998);
        traci.mapper.typename = default("VadereModuleMapper");
        traci.mapper.personType = default("crownet.nodes.ApplicationLayerPedestrian");
        traci.nodes.typename = default("VadereNodeManager");
        traci.nodes.personSinkModule = default(".mobility");
        traci.nodes.vehicleSinkModule = default(".mobility");
        traci.subscriptions.typename = default("VadereSubscriptionManager");

        storyboard.middlewareModule = default(".middleware");

    submodules:

		// implements IGeographicCoordinateSystem
        coordConverter: <default("OsgCoordConverterVadere")> like ICoordConverter if typename != "" {
            @display("p=524.8687,319.43625;i=block/table2;is=s");
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
            @display("p=523.74,207.69;is=s");
        }
        // LTE
        channelControl: LteChannelControl if hasCellular {
            @display("p=410.865,148.995;is=s");
        }
        binder: Binder if hasCellular {
            @display("p=411.99374,207.69;is=s");
        }
        carrierAggregation: CarrierAggregation if hasCellular {
            @display("p=411.99374,319.43625;is=s");
        }
        eNB[numEnb]: eNodeB if hasCellular && useNR == false {
            @display("p=39.50625,54.18");
        }
        gNB[numGnb]: gNodeB if hasCellular && useNR == true {
            @display("p=39.50625,54.18");
        }
        upf: Upf if hasCellular && useNR == true {
            @display("p=308.14874,190.75874;is=s");
        }
        iUpf: Upf if hasCellular && useNR == true {
            @display("p=308.14874,252.84;is=s");
        }
        server: StandardHost if hasCellularCoreNetwork {
            @display("p=308.14874,31.605;is=s;i=device/server");
        }
        router: Router if hasCellularCoreNetwork {
            @display("p=308.14874,89.17125;is=s;i=device/smallrouter");
        }
        pgw: PgwStandard if hasCellularCoreNetwork && useNR == false {
            @display("p=308.14874,136.57875;is=s");
        }
        // WLAN
        radioMedium: <default("Ieee80211ScalarRadioMedium")> like IRadioMedium if hasWlan {
            parameters:
                @display("p=410.865,267.51373;is=s");
        }
        // TraCI
        traci: <default("Manager")> like TraCiManager if typename != "" {
            parameters:
                @display("p=523.74,31.605;i=abstract/table;is=s");
        }
        identiyRegistry: IdentityRegistry {
            @display("p=523.74,259.6125;i=block/table2;is=s");
        }
        // Misc
        storyboard: <default("Storyboard")> like IStoryboard if hasStoryboard {
            parameters:
                @display("p=523.74,148.995;i=abstract/people;is=s");
        }
        gloablDensityMap: <default("")> like IGlobalDensityMap if typename != "" {
            @display("p=625.3275,31.605;i=block/table2;is=s");
        }

        flowController: <default("")> like IControlManager if typename != "" {
            @display("p=411.99374,391.67624;i=block/table2;is=s");
        }
        misc[numMisc]: <default("")> like INetworkNode if numMisc > 0 {

            @display("p=523.74,396.19125;is=vs");
        }
        fileWriterRegister: <default("")> like IFileWriterRegister if typename != "" {
            @display("p=308.14874,506.35;is=vs");
        }
        bonnMotionServer: <default("")> like BonnMotionServer if typename != "" {
            @display("p=449.2425,384.90375;i=block/table2_s;is=s");
        }

        //# submodule vector to store all pedestrians and vehicles - will be filled dynamically during simulations
        // pNode[0]: ApplicationLayerPedestrian;
        // vNode[0]: ApplicationLayerVehicle;


    connections:
        server.pppg++ <--> Eth10G <--> router.pppg++ if hasCellularCoreNetwork;
        router.pppg++ <--> Eth10G <--> pgw.filterGate if hasCellularCoreNetwork && useNR == false;
        router.pppg++ <--> Eth10G <--> upf.filterGate if hasCellularCoreNetwork && useNR == true;
        upf.pppg++ <--> Eth10G <--> iUpf.pppg++ if hasCellularCoreNetwork && useNR == true;

        for i=0..numEnb-1 {
            pgw.pppg++ <--> Eth10G <--> eNB[i].ppp if hasCellularCoreNetwork && hasCellular && useNR == false;
        }
        for i=0..numGnb-1 {
            iUpf.pppg++ <--> Eth10G <--> gNB[i].ppp if hasCellular && useNR == true;
        }

		//# X2 connections (full mesh)
        for i=0..numEnb-1, for j=0..numEnb-1 {
            eNB[i].x2++ <--> Eth10G <--> eNB[j].x2++ if i < j;
        }
        for i=0..numGnb-1, for j=0..numGnb-1 {
            gNB[i].x2++ <--> Eth10G <--> gNB[j].x2++ if i < j;
        }
}
