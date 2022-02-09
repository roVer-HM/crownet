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

import inet.node.inet.StandardHost;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth10G;
import inet.node.ethernet.Eth10M;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import simu5g.world.radio.ChannelControl;
import simu5g.common.binder.Binder;
import simu5g.common.carrierAggregation.CarrierAggregation;
import simu5g.nodes.Ue;
import simu5g.nodes.eNodeB;
import simu5g.world.radio.LteChannelControl;
import simu5g.nodes.PgwStandard;
import crownet.nodes.IMobileUe;
import crownet.nodes.UnequippedHost;


network LTE_d2d
{
    parameters:
        int numUeCell = default(0);
        int numUeD2D_a = default(0);
        int numUeD2D_b = default(0);
        int numMappings = default(0);
        string ueType = default("MobileUe");
        @figure[title](type=label; pos=0,-1; anchor=sw; color=darkblue);
        //@figure[rcvdPkText](type=indicatorText; pos=380,20; anchor=w; font=,10; textFormat="packets received: %g"; initialValue=0);

        @display("bgb=899,473");
    submodules:

        channelControl: LteChannelControl {
            @display("p=412,110;is=s");
        }
        routingRecorder: RoutingTableRecorder {
            @display("p=364,91;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=357,164;is=s");
            config = xmldoc("ip4Config.xml");
        }
        binder: Binder {
            @display("p=403,164;is=s");
        }
        carrierAggregation: CarrierAggregation {
            @display("p=450.37125,163.66875;is=s");
        }
        server: StandardHost {
            @display("p=315,150;is=s;i=device/server");
        }
        router: Router {
            @display("p=315,110;is=s;i=device/smallrouter");
        }
        pgw: PgwStandard {
            @display("p=314,43;is=l");
        }
        eNB: eNodeB {
            @display("p=37,51;is=vl");
        }
        ueCell[numUeCell]: Ue {
            @display("p=357,312;is=vs");
        }

        ueD2D_a[numUeD2D_a]: <ueType> like IMobileUe {
            @display("p=356,277;is=vs");
            ipv4.configurator.typename = "HostAutoConfigurator";
            ipv4.configurator.interfaces = "wlan";
        }

        ueD2D_b[numUeD2D_b]: <ueType> like IMobileUe {
            @display("p=356,245;is=vs");
            ipv4.configurator.typename = "HostAutoConfigurator";
            ipv4.configurator.interfaces = "wlan";
        }



    connections:
        server.pppg++ <--> Eth10G <--> router.pppg++;
        router.pppg++ <--> Eth10G <--> pgw.filterGate;
        pgw.pppg++ <--> Eth10G <--> eNB.ppp;
}
