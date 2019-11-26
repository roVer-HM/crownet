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


package rover.simulations.muenchnerFreiheit.stsc;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
import inet.environment.common.PhysicalEnvironment;
import org.car2x.veins.subprojects.veins_inet.VeinsInetPerson;
import org.car2x.veins.subprojects.veins_inet.vadere.TraCIScenarioManagerVadere;
import inet.node.inet.StandardHost;


network MucFreiNet80211
{
    parameters:
        int numHosts = default(0);
        @figure[title](type=label; pos=0,-1; anchor=sw; color=darkblue);
		@figure[rcvdPkText](type=indicatorText; pos=380,20; anchor=w; font=,18; textFormat="packets received: %g"; initialValue=0);
       
        
    submodules:
        visualization: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            @display("p=300,165");           
        }

        physicalEnvironment: PhysicalEnvironment {

            @display("p=300,281");
        }

        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=300,100;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=299,229;is=s");
        }
        manager: TraCIScenarioManagerVadere {
            @display("p=300,38");
        }
		
		rsu : VeinsInetPerson {
			@display("i=misc/smartphone;is=vs");
//			numWlanInterfaces = 0;
		}

        p[numHosts]: VeinsInetPerson {
            @display("i=misc/smartphone;is=vs;p=349,38");
        }
}
