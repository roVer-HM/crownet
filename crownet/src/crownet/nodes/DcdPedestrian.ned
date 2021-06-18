package crownet.nodes;

import crownet.aid.IAidLayer;
import inet.applications.contract.IApp;
import inet.transportlayer.contract.ITcp;
import inet.transportlayer.contract.IUdp;
import inet.transportlayer.contract.ISctp;
import crownet.applications.common.IBaseApp;
import crownet.applications.dmap.IDensityMapApp;
import crownet.applications.beacon.IBeaconApp;
import inet.common.MessageDispatcher;

module DcdPedestrian extends TransportLayerRoadUser
{
    parameters:
        @figure[applicationLayer](type=rectangle; pos=250,6; size=600,130; lineColor=#808080; cornerRadius=5; fillColor=#ffff00; fillOpacity=0.1);
        @figure[applicationLayer.title](type=text; pos=845,11; anchor=ne; text="application layer");

        @figure[aidLayer](type=rectangle; pos=250,156; size=600,130; lineColor=#808080; cornerRadius=5; fillColor=#BDBDD6; fillOpacity=0.1);
        @figure[aidLayer.title](type=text; pos=845,161; anchor=ne; text="adaptive information dissemination layer");

        int numApps = default(0);   // no of apps.
                                    // Specify the app types in INI file with app[0..1].typename="UdpVideoStreamClient" syntax
        //# Aid
        bool useAid = default(true);
        string aidType = ifTrueOrElse(useAid, "SimpleAidLayer", "");
        //# Decentralized Crowed Density 
        string crownetAppType = ifTrueOrElse(useAid, "crownet.applications.common.CrownetAidApp", "crownet.applications.common.CrownetUdpApp");
        densityMap.app.typename = default("DensityMapAppSimple");
        densityMap.socket.destPort = default(30001);
        densityMap.socket.localPort = default(30001);
        beacon.app.typename = default("BeaconSimple");
        beacon.socket.destPort = default(30002);
        beacon.socket.localPort = default(30002);

    submodules:
        app[numApps]: <> like IApp {
            @display("p=346.528,54.88,row,140");
        }

        densityMap: <default(crownetAppType)> like IBaseApp if typename!="" {
            parameters:
                @display("p=605.248,54.88;i=block/users;is=s");
        }
        beacon: <default(crownetAppType)> like IBaseApp if typename!="" {
            parameters:
                @display("p=771.456,54.88;i=block/broadcast;is=s");
        }

        aid: <default(aidType)> like IAidLayer if typename !="" {
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
        densityMap.socketIn <-- aa.out++ if exists(densityMap);
        densityMap.socketOut --> aa.in++ if exists(densityMap);

        beacon.socketIn <-- aa.out++ if exists(beacon);
        beacon.socketOut --> aa.in++ if exists(beacon);

        aa.out++ --> aid.upperIn if exists(aid);
        aa.in++ <-- aid.upperOut if exists(aid);

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
