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


package  crownet.test.omnetpp.lib;

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
import simu5g.world.radio.LteChannelControl;
import simu5g.nodes.PgwStandard;
import crownet.nodes.IMobileUe;
import crownet.nodes.UnequippedHost;
import crownet.common.ICoordConverter;
import crownet.common.IGlobalDensityMap;
import crownet.control.IControlManager;
import crownet.common.util.IFileWriterRegister;



network TestStationaryWorld
{
    parameters:
        @figure[title](type=label; pos=0,-1; anchor=sw; color=darkblue);
        @display("bgb=899,473");

        bool hasCellular = default(true);
        bool hasCellularCoreNetwork = default(hasCellular);
        int numMisc = default(0);
        int numEnb = default(1);
        coordConverter.xBound = default(50m);
        coordConverter.yBound = default(50m);

    submodules:

		// implements IGeographicCoordinateSystem
        coordConverter: <default("OsgCoordConverterLocal")> like ICoordConverter if typename != "" {
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
        eNB[numEnb]: eNodeB if hasCellular {
            @display("p=39.50625,54.18");
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
        gloablDensityMap: <default("")> like IGlobalDensityMap if typename != "" {
            @display("p=308.14874,268.6425;i=block/table2;is=s");
        }
        misc[numMisc]: <default("")> like INetworkNode if numMisc > 0 {

            @display("p=308.14874,406.35;is=vs");
        }
        fileWriterRegister: <default("")> like IFileWriterRegister if typename != "" {
           @display("p=308.14874,506.35;is=vs");
        }


    connections:
        server.pppg++ <--> Eth10G <--> router.pppg++ if hasCellularCoreNetwork;
        router.pppg++ <--> Eth10G <--> pgw.filterGate if hasCellularCoreNetwork;
        for i=0..numEnb-1 {
            pgw.pppg++ <--> Eth10G <--> eNB[i].ppp if hasCellularCoreNetwork;
        }

		//# X2 connections (full mesh)
        for i=0..numEnb-1, for j=0..numEnb-1 {
            eNB[i].x2++ <--> Eth10G <--> eNB[j].x2++ if i < j;
        }

}
