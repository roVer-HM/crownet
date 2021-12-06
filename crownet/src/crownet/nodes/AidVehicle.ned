package crownet.nodes;

import inet.applications.contract.IApp;
import inet.transportlayer.contract.ITcp;
import inet.transportlayer.contract.IUdp;
import inet.transportlayer.contract.ISctp;
import inet.common.MessageDispatcher;
import crownet.aid.IAidLayer;

module AidVehicle extends TransportLayerRoadUser 
{
    parameters:
        @display("i=crownet/node/car;is=vs;bgb=860,780");
        @figure[applicationLayer](type=rectangle; pos=250,6; size=600,280; lineColor=#808080; cornerRadius=5; fillColor=#ffff00; fillOpacity=0.1);
        @figure[applicationLayer.title](type=text; pos=845,11; anchor=ne; text="application layer");
        
        int numApps = default(0);   // no of apps.
                                    // Specify the app types in INI file with app[0..1].typename="UdpVideoStreamClient" syntax
   
   		middleware.typename = default(ifTrueOrElse(useArtery, "VehicleMiddleware", "VehicleMiddlewareEmpty"));
   		mobility.typename = default("artery.inet.VehicleMobility");
        vanetza.position.typename = default("VehiclePositionProvider");
        
    submodules:
        app[numApps]: <> like IApp {
            @display("p=346.528,54.88,row,140");
        }

        aid: <default("SimpleAidLayer")> like IAidLayer if typename !="" {
            @display("p=396.704,216.384");
        }

        // message dispatcher for SAP between application and transport layer
        aa: MessageDispatcher {
            parameters:
                @display("p=550,146;b=600,5,,,,1");
        }
        // message dispatcher for SAP between application and transport layer
        at: MessageDispatcher {
            parameters:
                @display("p=550,296;b=600,5,,,,1");
        }
 
    connections allowunconnected:
        //#
        //# Internal Tcp/Udp applications connections with AID service
        //#

        for i=0..numApps-1 {
            app[i].socketOut --> aa.in++;
            app[i].socketIn <-- aa.out++;
        }

        aa.out++ --> aid.upperIn;
        aa.in++ <-- aid.upperOut;

        at.out++ --> aid.lowerIn;
        at.in++ <-- aid.lowerOut;

        at.out++ --> udp.appIn if hasUdp;
        at.in++ <-- udp.appOut if hasUdp;

        at.out++ --> tcp.appIn if hasTcp;
        at.in++ <-- tcp.appOut if hasTcp;

        at.out++ --> sctp.appIn if hasSctp;
        at.in++ <-- sctp.appOut if hasSctp;

        at.out++ --> tn.in++;
        at.in++ <-- tn.out++;

        aa.out++ --> at.in++;
        aa.in++ <-- at.out++;  
}