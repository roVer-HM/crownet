package crownet.simulations.networks;

import crownet.simulations.networks.LabSingleCellBase;

import simu5g.nodes.Upf;
import simu5g.nodes.NR.gNodeB;
import simu5g.nodes.NR.NRUe;

import inet.common.misc.ThruputMeteringChannel;

network LabSingleCellNR extends LabSingleCellBase
{   
    parameters:
        int numUe = default(1);
        @display("i=block/network2;bgb=7.5,10,#FCE94F;bgi=background/lab_r1009a_map,s;bgtt=Lab (5G NR) R1.009a (Munich University of Applied Sciences);bgg=1,,#888A85;bgu=m");
        
    types:
    	channel Eth1GMetered extends ThruputMeteringChannel
        {
            datarate = 1024Mbps;
            thruputDisplayFormat = "u";
        }
        
    submodules:
        gNB: gNodeB {
            @display("p=6,2.6142879;is=vl");
        }
        upf: Upf {
            @display("p=6,4.4116106");
        }
        ue[numUe]: NRUe {
            @display("p=6,3");
        }
    
        connections:
        server.pppg++ <--> Eth1GMetered <--> router.pppg++;
        router.pppg++ <--> Eth1GMetered <--> upf.filterGate;
        upf.pppg++    <--> Eth1GMetered <--> gNB.ppp;
}
