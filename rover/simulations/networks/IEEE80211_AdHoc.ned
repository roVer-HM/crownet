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


import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import org.car2x.veins.subprojects.veins_inet.VeinsInetPerson;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
import org.car2x.veins.modules.mobility.traci.ITraCIManager;

network IEEE80211_AdHoc
{
    parameters:
        int numStationary = default(0);
        int numMobile = default(0);

    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=298,23;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=217,65;is=s");
        }
        station[numStationary]: AdhocHost {
            parameters:
                @display("r=,,#707070;p=453,75;i=device/accesspoint_s;t1=t");
        }
        hostMobile[numMobile]: VeinsInetPerson {
            parameters:
                @display("r=,,#707070;p=453,23;i=veins/node/pedestrian_s");
        }
        manager: <default(TraCIScenarioManagerVadere)> like ITraCIManager if typename != "" {
            @display("p=218,23;is=s;i=block/square");
        }

}
