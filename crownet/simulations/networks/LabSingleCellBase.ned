package crownet.simulations.networks;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth10G;
import inet.node.ethernet.Eth10M;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import simu5g.common.binder.Binder;
import simu5g.common.carrierAggregation.CarrierAggregation;
import simu5g.world.radio.LteChannelControl;

network LabSingleCellBase
{   
    submodules:
        channelControl: LteChannelControl {
            @display("p=8,1;is=s");
        }
        routingRecorder: RoutingTableRecorder {
            @display("p=8,2;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=8,3");
        }
        binder: Binder {
            @display("p=8,4;is=s");
        }
        carrierAggregation: CarrierAggregation {
            @display("p=8,5;is=s");
        }
        router: Router {
            @display("p=1.0457151,5.8167906;i=device/smallrouter");
        }
        server: StandardHost {
            @display("p=1.0457151,7;is=n;i=device/server");
        }
}
