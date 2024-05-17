//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//
package crownet.simulations.networks;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth10G;
import inet.node.ethernet.Eth10M;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.common.misc.ThruputMeteringChannel;
import simu5g.common.binder.Binder;
import simu5g.common.carrierAggregation.CarrierAggregation;
import simu5g.nodes.Ue;
import simu5g.nodes.eNodeB;
import simu5g.world.radio.LteChannelControl;
import simu5g.nodes.PgwStandard;

import inet.common.misc.ThruputMeteringChannel;


network LabSingleCellLte extends LabSingleCellBase
{
    	parameters:
        	int numUe = default(1);
        	int numENB = default(1);
        	@display("i=block/network2;bgb=7.5,10,#FCE94F;bgi=background/lab_r1009a_map,s;bgtt=Lab (LTE-A setup), R1.009a, HM Munich University of Applied Sciences;bgg=1,,#888A85;bgu=m");
    	
        types:
            channel Eth1GMetered extends ThruputMeteringChannel
            {
                datarate = 1024Mbps;
                thruputDisplayFormat = "u";
            }
        submodules:
            eNB[numENB]: eNodeB {
            	@display("p=1.0457151,2.6142879;is=vl");
        	}
        	pgw: PgwStandard {
            	@display("p=1.0293758,4.4116106;is=l");
        	}
        	ue[numUe]: Ue {
            	@display("p=6,3");
        	}
        	
        connections:
        server.pppg++ <--> Eth1GMetered <--> router.pppg++;
        router.pppg++ <--> Eth1GMetered <--> pgw.filterGate;
        for i=0..numENB-1 {
            pgw.pppg++ <--> Eth1GMetered <--> eNB[i].ppp;
        }
}
