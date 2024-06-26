package crownet.nodes;

import crownet.aid.IAidLayer;
import inet.applications.contract.IApp;
import inet.transportlayer.contract.ITcp;
import inet.transportlayer.contract.IUdp;
import inet.transportlayer.contract.ISctp;
import inet.common.MessageDispatcher;

module AidPedestrian extends TransportLayerRoadUser
{
    parameters:
        @display("i=crownet/node/pedestrian;is=vs;bgb=860,780");
        @figure[applicationLayer](type=rectangle; pos=250,6; size=600,130; lineColor=#808080; cornerRadius=5; fillColor=#ffff00; fillOpacity=0.1);
        @figure[applicationLayer.title](type=text; pos=845,11; anchor=ne; text="application layer");

        @figure[aidLayer](type=rectangle; pos=250,156; size=600,130; lineColor=#808080; cornerRadius=5; fillColor=#BDBDD6; fillOpacity=0.1);
        @figure[aidLayer.title](type=text; pos=845,161; anchor=ne; text="adaptive information dissemination layer");

        int numApps = default(0);   // no of apps.
                                    // Specify the app types in INI file with app[0..1].typename="UdpVideoStreamClient" syntax
        
        //# Use vadere for pedestrian simulation? If false, sumo is assumed.
        bool useVadere = default(true);
       
        middleware.typename = default(ifTrueOrElse(useVadere,
            ifTrueOrElse(useArtery, "PedestrianMiddleware", "PedestrianMiddlewareEmpty"),
            ifTrueOrElse(useArtery, "PersonMiddleware", "PersonMiddlewareEmpty")));
        mobility.typename = default(ifTrueOrElse(useVadere, "crownet.artery.traci.InetVaderePersonMobility", "artery.inet.PersonMobility"));
        vanetza.position.typename = default(ifTrueOrElse(useVadere, "PedestrianPositionProvider", "PersonPositionProvider"));
        
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
