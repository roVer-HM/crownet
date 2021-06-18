package crownet.nodes;

import inet.applications.contract.IApp;
import inet.transportlayer.contract.ITcp;
import inet.transportlayer.contract.IUdp;
import inet.transportlayer.contract.ISctp;
import inet.common.MessageDispatcher;

module ApplicationLayerPedestrian extends TransportLayerRoadUser 
{
    parameters:
        @display("i=crownet/node/pedestrian;is=vs;bgb=860,780");
        @figure[applicationLayer](type=rectangle; pos=250,6; size=600,280; lineColor=#808080; cornerRadius=5; fillColor=#ffff00; fillOpacity=0.1);
        @figure[applicationLayer.title](type=text; pos=845,11; anchor=ne; text="application layer");
        
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

        // message dispatcher for SAP between application and transport layer
        at: MessageDispatcher {
            parameters:
                @display("p=550,296;b=600,5,,,,1");
        }
 
    connections allowunconnected:
        //#
        //# Internal Tcp/Udp/Sctp to applications connections 
        //#

        for i=0..numApps-1 {
            app[i].socketOut --> at.in++;
            app[i].socketIn <-- at.out++;
        }

        at.out++ --> udp.appIn if hasUdp;
        at.in++ <-- udp.appOut if hasUdp;

        at.out++ --> tcp.appIn if hasTcp;
        at.in++ <-- tcp.appOut if hasTcp;

        at.out++ --> sctp.appIn if hasSctp;
        at.in++ <-- sctp.appOut if hasSctp;

        at.out++ --> tn.in++;
        at.in++ <-- tn.out++;   
}