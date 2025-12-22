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

import crownet.simulations.networks.LabSingleCellBase;

import inet.visualizer.contract.IIntegratedVisualizer;
import inet.environment.common.PhysicalEnvironment;
import inet.node.inet.StandardHost;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth10G;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadioMedium;
import inet.node.contract.INetworkNode;
import inet.visualizer.contract.ISceneVisualizer;
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
import crownet.common.DensityMapSceneCanvasVisualizer;
import crownet.nodes.ApplicationLayerPedestrian;

import artery.storyboard.IStoryboard;
import artery.utility.IdentityRegistry;

import inet.common.misc.ThruputMeteringChannel;


// note: we do not use the LabSingleCellBase here, since the cellular infrastructure is not instantiated if only WLAN is active
network LabSingleCellCrowNet
{
    parameters:
        @display("i=block/network2;bgb=8,10,#FCE94F;bgi=background/lab_r1009a_map,s;bgtt=Lab (CrowNet) R1.009a (Munich University of Applied Sciences);bgg=1,,#888A85;bgu=m");
        int numUe = default(1);
        int numGnb = default(1);
        bool hasCellular = default(false);
        bool hasCellularCoreNetwork = default(hasCellular);
        bool hasStoryboard = default(false);
        bool hasWlan = default(true);
        storyboard.middlewareModule = default(".middleware");

    types:
        channel Eth10GMetered extends ThruputMeteringChannel
        {
            datarate = 10240Mbps;
            thruputDisplayFormat = "u";
        }

    submodules:
        coordConverter: <default("OsgCoordConverterLocal")> like ICoordConverter if typename != "" {
            @display("p=4.74,8.18;i=block/table2;is=s");
        }
        visualization: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            @display("p=2.79,1.08;is=s");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=9,4;is=s");
        }
        routingRecorder: RoutingTableRecorder {
            @display("p=4.74,0.22;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=6.2,0.22;is=s");
        }

        // cellular infrastructure - only instantiated if required
        channelControl: LteChannelControl if hasCellular {
            @display("p=4.45,0.99;is=s");
        }
        binder: Binder if hasCellular {
            @display("p=3.56,8;is=s");
        }
        carrierAggregation: CarrierAggregation if hasCellular {
            @display("p=2.39,8.19;is=s");
        }
        gNB[numGnb]: gNodeB if hasCellular {
            @display("p=3.7,4.33;is=vl");
        }
        upf: Upf if hasCellular {
            @display("p=2.5,5.97");
        }
        iUpf: Upf if hasCellular {
            @display("p=2.5,6.89;is=s");
        }
        server: StandardHost if hasCellularCoreNetwork {
            @display("p=2.5,4.11;is=s;i=device/server");
        }
        router: Router if hasCellularCoreNetwork {
            @display("p=2.5,5.07;is=s;i=device/smallrouter");
        }
        identiyRegistry: IdentityRegistry {
            @display("i=block/table2;is=s;p=1.05,9.44");
        }

        storyboard: <default("Storyboard")> like IStoryboard if hasStoryboard {
            parameters:
                @display("p=0.78,8.9;i=abstract/people;is=s");
        }

        globalDensityMap: <default("")> like IGlobalDensityMap if typename != "" {
            @display("p=6.5,9.15;i=block/table2;is=s");
        }
        globalMeasurementMap: <default("")> like IGlobalDensityMap if typename != "" {
            @display("p=4.85,9.35;i=block/table2;is=s");
        }

        flowController: <default("")> like IControlManager if typename != "" {
            @display("p=5.7,7.88;i=block/table2;is=s");
        }

        fileWriterRegister: <default("")> like IFileWriterRegister if typename != "" {
            @display("p=2.29,8.9;is=s");
        }

        mapVisualizer: <default("DensityMapSceneCanvasVisualizer")> like ISceneVisualizer if typename != "" {
            @display("p=1.34,0.99;is=s");
        }

        // WLAN
        radioMedium: <default("Ieee80211ScalarRadioMedium")> like IRadioMedium if hasWlan {
            parameters:
                @display("p=1.57,7.73;is=s");
        }

        ue[numUe]: <default("ApplicationLayerPedestrian")> like INetworkNode if numUe > 0 {
            @display("p=6,3");
        }

    connections:
        server.pppg++ <--> Eth10GMetered <--> router.pppg++ if hasCellularCoreNetwork;
        router.pppg++ <--> Eth10GMetered <--> upf.filterGate if hasCellularCoreNetwork;
        upf.pppg++ <--> Eth10GMetered <--> iUpf.pppg++ if hasCellularCoreNetwork;
        for i=0..numGnb-1 {
            iUpf.pppg++ <--> Eth10GMetered <--> gNB[i].ppp if hasCellularCoreNetwork;
        }
}
