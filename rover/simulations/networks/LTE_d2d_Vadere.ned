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
import org.car2x.veins.modules.mobility.traci.ITraCIManager;
import org.car2x.veins.subprojects.veins_inet.mapping.NodeMappingDistribution;



network LTE_d2d_Vadere extends LTE_d2d
{
    parameters:
        manager.typename = "TraCIScenarioManagerVadere";

    submodules:
        visualization: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            @display("p=366,114;is=s");
        }

        physicalEnvironment: PhysicalEnvironment {
            @display("p=366,62;is=s");
        }
        
        mappingDistribution[numMappings]: NodeMappingDistribution {
            @display("p=357,207;is=s;i=block/square");
        }
        
        manager: <default(TraCIScenarioManagerVadere)> like ITraCIManager if typename != "" {
            @display("p=364,25;is=s;i=block/square");
        }
}
