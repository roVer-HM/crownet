// 
//                           SimuLTE
// 
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself, 
// and cannot be removed from it.
// 
package rover.simulations.networks;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth10G;
import inet.node.ethernet.Eth10M;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.common.misc.ThruputMeteringChannel;
import lte.corenetwork.binder.LteBinder;
import lte.corenetwork.nodes.Ue;
import lte.corenetwork.nodes.eNodeB;
import lte.world.radio.LteChannelControl;
import lte.epc.PgwStandardSimplified;


network LabSingleCellBase
{
    parameters:
        int numUe = default(1);
        @display("i=block/network2;bgb=7.5,10,#FCE94F;bgi=background/lab_r1009a_map,s;bgtt=Lab R1.009a (Munich University of Applied Sciences);bgg=1,,#888A85;bgu=m");       
          
    submodules:
        channelControl: LteChannelControl {
            @display("p=8,1;is=s");
        }
        routingRecorder: RoutingTableRecorder {
            @display("p=8,2;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=8,3");
            config = xmldoc("demo.xml");
        }
        binder: LteBinder {
            @display("p=8,4;is=s");
        }
        eNB: eNodeB {
            @display("p=1.0457151,2.6142879;is=vl");
        }
        pgw: PgwStandardSimplified {
            nodeType = "PGW";
            @display("p=1.0293758,4.4116106;is=l");
        }
        router: Router {
            @display("p=1.0457151,5.8167906;i=device/smallrouter");
        }
        server: StandardHost {
            @display("p=1.0457151,7;is=n;i=device/server");
        }
        ue[numUe]: Ue {
            @display("p=6,3");
        }
}

network LabSingleCell extends LabSingleCellBase
{
        types:
            channel Eth1GMetered extends ThruputMeteringChannel
            {
                datarate = 1024Mbps;
                thruputDisplayFormat = "u";
            }
    
        connections:
        server.pppg++ <--> Eth1GMetered <--> router.pppg++;
        router.pppg++ <--> Eth1GMetered <--> pgw.filterGate;
        pgw.pppg++ <--> Eth1GMetered <--> eNB.ppp; 
}

network LabSingleCellAdaptedRB25 extends LabSingleCellBase
{
        types:
            channel Eth1GMeteredAdapted extends ThruputMeteringChannel
            {
                // adapted to lab measurements with 25 RBs: approx. 500us
                delay = 500us;
                datarate = 1024Mbps;
                thruputDisplayFormat = "u";
            }
        connections:
        server.pppg++ <--> Eth1GMeteredAdapted <--> router.pppg++;
        router.pppg++ <--> Eth1GMeteredAdapted <--> pgw.filterGate;
        pgw.pppg++ <--> Eth1GMeteredAdapted <--> eNB.ppp; 
}

network LabSingleCellAdaptedRB50 extends LabSingleCellBase
{
        types:
            channel Eth1GMeteredAdapted extends ThruputMeteringChannel
            {
                // adapted to lab measurements with 50 RBs: approx. 733us
                delay = 733us;
                datarate = 1024Mbps;
                thruputDisplayFormat = "u";
            }
        connections:
        server.pppg++ <--> Eth1GMeteredAdapted <--> router.pppg++;
        router.pppg++ <--> Eth1GMeteredAdapted <--> pgw.filterGate;
        pgw.pppg++ <--> Eth1GMeteredAdapted <--> eNB.ppp; 
}
