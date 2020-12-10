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

import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
import inet.environment.common.PhysicalEnvironment;
import inet.node.inet.StandardHost;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import rover.common.ICoordConverter;
import rover.common.GlobalDensityMap;

import artery.storyboard.Storyboard;
import artery.utility.IdentityRegistry;
import traci.Manager;


network World
{
    parameters:
        @figure[title](type=label; pos=0,-1; anchor=sw; color=darkblue);
        @display("bgb=899,473");

        bool withStoryboard = default(false);


    submodules:

        coordConverter: <default("OsgCoordConverterRemote")> like ICoordConverter if typename != "" {
            @display("p=411.99374,148.995;i=block/table2;is=s");
        }

        visualization: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            @display("p=540.67126,89.17125;is=s");
        }

        physicalEnvironment: PhysicalEnvironment {
            @display("p=413,32;is=s");
        }
        routingRecorder: RoutingTableRecorder {
            @display("p=411.99374,89.17125;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=540.67126,198.66;is=s");
            config = xmldoc("ip4Config.xml");
        }

        traci: Manager {
            parameters:
                @display("p=540.67126,31.605");
        }

        storyboard: Storyboard if withStoryboard {
            parameters:
                @display("p=540.67126,148.995");
        }

        gloablDensityMap: GlobalDensityMap {
            @display("p=411.99374,198.66;i=block/table2;is=s");
        }

        identiyRegistry: IdentityRegistry {
            @display("p=540.67126,258.48373;i=block/table2;is=s");
        }

}
