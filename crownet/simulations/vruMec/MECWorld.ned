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

package crownet.simulations.vruMec;

import inet.visualizer.contract.IIntegratedVisualizer;
import artery.storyboard.IStoryboard;
import traci.Manager;
import traci.TraCiManager;
import artery.utility.IdentityRegistry;

import crownet.common.ICoordConverter;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth10G;
import simu5g.world.radio.LteChannelControl;
import simu5g.common.binder.Binder;
import simu5g.common.carrierAggregation.CarrierAggregation;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import simu5g.nodes.Upf;
import simu5g.nodes.NR.gNodeB;
import simu5g.nodes.NR.NRUe;
import simu5g.nodes.mec.MECHost;
import inet.environment.common.PhysicalEnvironment;

import crownet.common.util.CrownetMobilityVisualizer;

import simu5g.nodes.mec.MECOrchestrator.MecOrchestrator;
import simu5g.nodes.mec.UALCMP.UALCMP;

network MecWorld
{
    @display("bgb=1380.5399,1027.62");

    submodules:
        coordConverter: <default("OsgCoordConverterVadere")> like ICoordConverter if typename != "" {
            @display("p=1301.7375,145.725;i=block/table2;is=s");
        }
        visualization: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            @display("p=1301.7375,23.925;is=s");
        }
        net_visualization: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            @display("p=1301.7375,84.825;is=s");
        }
        traci: <default("Manager")> like TraCiManager if typename != "" {
            parameters:
                @display("p=1301.7375,272.9625");
        }
        identiyRegistry: IdentityRegistry {
            @display("p=1301.7375,345.825;i=block/table2;is=s");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=1159.275,217.5;is=s");
        }

        carrierAggregation: CarrierAggregation {
            @display("p=1159.275,23.925;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=1159.275,145.725");
        }
        channelControl: LteChannelControl {
            @display("p=1300.65,203.3625;is=s");
        }
        binder: Binder {
            @display("p=1159.275,84.825;is=s");
        }
        gNodeB1: gNodeB {
            @display("p=352.60202,480.446;is=l");
        }
        mecHost: MECHost {
            @display("p=588.2,762.93");
        }
        ualcmp: UALCMP {
            @display("p=55.359997,98.61");
        }
        upf: Upf {
            @display("p=55.359997,231.81999");
        }
        iUpf: Upf {
            @display("p=515.54,762.93");
        }
        mecOrchestrator: MecOrchestrator {
            @display("p=230.09,98.61");
        }

        mobilityVisualizer: CrownetMobilityVisualizer;
        	
    connections:
        upf.pppg++ <--> Eth10G <--> iUpf.pppg++;
        iUpf.pppg++ <--> Eth10G <--> gNodeB1.ppp;
        ualcmp.ppp++ <--> Eth10G <--> upf.filterGate;

        ualcmp.toMecOrchestrator --> mecOrchestrator.fromUALCMP;
        ualcmp.fromMecOrchestrator <-- mecOrchestrator.toUALCMP;

        mecHost.ppp++ <--> Eth10G <--> iUpf.pppg++;
}
