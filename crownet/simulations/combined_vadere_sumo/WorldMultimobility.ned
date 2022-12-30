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


package crownet.simulations.combined_vadere_sumo;

import crownet.simulations.networks.World;

import crownet.common.ICoordConverter;
import traci.Manager;
import traci.TraCiManager;


network MultimobilityWorld extends World
{
    parameters:
        int numAdditionalTraci = default(1);
        int numAdditionalCoordConverters = default(1);
        
        additionalTraci.core.typename = default("VadereCore");
        additionalTraci.core.version = default(-1);
        additionalTraci.core.selfStopping = default(false);
        additionalTraci.launcher.typename = default("VadereLauncher");
        additionalTraci.launcher.hostname = default("vadere");
        additionalTraci.launcher.port = default(9998);
        additionalTraci.mapper.typename = default("VadereModuleMapper");
        additionalTraci.mapper.personType = default("crownet.nodes.ApplicationLayerPedestrian");
        additionalTraci.nodes.typename = default("VadereNodeManager");
        additionalTraci.nodes.personSinkModule = default(".mobility");
        additionalTraci.nodes.vehicleSinkModule = default(".mobility");
        additionalTraci.subscriptions.typename = default("VadereSubscriptionManager");

    submodules:
        additionalCoordConverter[numAdditionalCoordConverters]: <default("OsgCoordConverterVadere")> like ICoordConverter {
            @display("p=524.8687,319.43625;i=block/table2;is=s");
        }
        additionalTraci[numAdditionalTraci]: <default("Manager")> like TraCiManager {
            parameters:
                @display("p=523.74,31.605;i=abstract/table;is=s");
        }

}
