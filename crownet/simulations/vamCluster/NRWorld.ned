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

package crownet.simulations.vamCluster;

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

import simu5g.nodes.mec.MECOrchestrator.MecOrchestrator;
import simu5g.nodes.mec.UALCMP.UALCMP;

network World
{
    parameters:
        @display("bgb=748.50604,837.172");
        int numTraci = default(1);

    submodules:
        coordConverter[numTraci]: <default("OsgCoordConverterVadere")> like ICoordConverter {
            @display("p=150,337;i=block/table2;is=s");
        }
        visualization: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            @display("p=150,524;is=s");
        }
        net_visualization: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            @display("p=150,479;is=s");
        }
        traci[numTraci]: <default("Manager")> like TraCiManager {
            parameters:
                @display("p=150.526,148.464");
        }
        identiyRegistry: IdentityRegistry {
            @display("p=150,203;i=block/table2;is=s");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=150,295;is=s");
        }

        carrierAggregation: CarrierAggregation {
            @display("p=150,432;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=150.526,68.046005");
        }
        channelControl: LteChannelControl {
            @display("p=148,250;is=s");
        }
        binder: Binder {
            @display("p=150,384;is=s");
        }
        gNodeB1: gNodeB {
            @display("p=451,211;is=l");
        }
        upf: Upf {
            @display("p=451,107");
        }
    connections:
        upf.pppg++ <--> Eth10G <--> gNodeB1.ppp;
}
