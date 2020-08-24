package rover.artery;

import rover.artery.PedestrianMiddleware;
import rover.artery.IDispatchRadioDriver;
import artery.networking.Vanetza;
import artery.networking.IVanetza;
import artery.inet.PlainVehicle;
import artery.nic.IRadioDriver;
import artery.application.IMiddleware;
import inet.node.inet.INetworkNode;
import inet.linklayer.contract.IWirelessInterface;
import inet.mobility.contract.IMobility;
import inet.networklayer.common.InterfaceTable;
import rover.aid.IAidLayer;
import inet.networklayer.configurator.ipv4.HostAutoConfigurator;
import inet.applications.contract.IApp;
import inet.mobility.contract.IMobility;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.contract.IRoutingTable;
import inet.networklayer.contract.INetworkLayer;
import inet.transportlayer.contract.ITcp;
import inet.transportlayer.contract.IUdp;
import inet.transportlayer.contract.ISctp;
import inet.common.MessageDispatcher;
import lte.stack.ILteNic;

module PedestrianFull like INetworkNode
{
    parameters:
        @networkNode();
        @labels(node,ethernet-node,wireless-node);
        @display("i=rover/node/pedestrian;is=vs;bgb=860,780");
        @figure[applicationLayer](type=rectangle; pos=250,6; size=600,130; lineColor=#808080; cornerRadius=5; fillColor=#ffff00; fillOpacity=0.1);
        @figure[applicationLayer.title](type=text; pos=845,11; anchor=ne; text="application layer");

        @figure[aidLayer](type=rectangle; pos=250,156; size=600,130; lineColor=#808080; cornerRadius=5; fillColor=#BDBDD6; fillOpacity=0.1);
        @figure[aidLayer.title](type=text; pos=845,161; anchor=ne; text="adaptive information dissemination layer");

        @figure[transportLayer](type=rectangle; pos=250,306; size=600,130; fillColor=#ff0000; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[transportLayer.title](type=text; pos=845,311; anchor=ne; text="transport layer");

        @figure[networkLayer](type=rectangle; pos=250,456; size=600,130; fillColor=#00ff00; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[networkLayer.title](type=text; pos=845,461; anchor=ne; text="network layer");

        @figure[linkLayer](type=rectangle; pos=250,606; size=600,130; fillColor=#0000ff; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[linkLayer.title](type=text; pos=845,611; anchor=ne; text="link layer");
        @figure[submodules];

        @statistic[posX](source="xCoord(mobilityPos(mobilityStateChanged))"; record=vector?);
        @statistic[posY](source="yCoord(mobilityPos(mobilityStateChanged))"; record=vector?);
        int numRadios = default(1);
        bool withAntennaMobility = default(false);
        *.mobility.visualRepresentation = "^";


        //# AID layer
        string aidType = default("SimpleAidLayer");

        //# Mobility
        string mobilityType = default("rover.artery.traci.MobilityRoverArtery");

        //# Apps
        int numApps = default(0);   // no of apps.
                                    // Specify the app types in INI file with app[0..1].typename="UdpVideoStreamClient" syntax 

        //# Transport layer
        bool hasUdp = default(firstAvailableOrEmpty("Udp") != "");
        bool hasTcp = default(firstAvailableOrEmpty("Tcp", "TcpLwip", "TcpNsc") != "");
        bool hasSctp = default(false);
        bool hasGeoNet = default(true);

        //# Node specs
        string nodeType = "UE";  // DO NOT CHANGE
        int masterId;
        int macNodeId = default(0); // TODO: this is not a real parameter
        int macCellId = default(0); // TODO: this is not a real parameter

        //# D2D capability
        string nicType = default("LteNicUe");

        //# Network Layer specs
        bool hasIpv4 = default(true);
        bool hasIpv6 = default(false);

        **.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default("^.ipv4.routingTable");

    gates:
        input radioIn @directIn;     // connection to master

    submodules:
        interfaceTable: InterfaceTable {
            @display("p=103,194;is=s");
        }
        mobility: <default("rover.artery.traci.MobilityRoverArtery")> like IMobility {
            parameters:
                @display("p=48.608,101.92");
        }
        middleware: <default("PedestrianMiddleware")> like IMiddleware if hasGeoNet {
            parameters:
                @display("p=109.76,296.352");
                routerModule = ".vanetza.router";
                mobilityModule = ".mobility";
        }
        vanetza: <default("Vanetza")> like IVanetza if hasGeoNet {
            parameters:
                @display("p=109.76,393.568");
                *.middlewareModule = absPath("^.middleware");
                *.mobilityModule = absPath("^.mobility");
                *.radioDriverModule = absPath("^.radioDriver[0]");
                runtime.datetime = middleware.datetime;
                *.position.typename = "PedestrianPositionProvider";
        }
        radioDriver: <default("LteRadioDriver")> like IDispatchRadioDriver if hasGeoNet {
            parameters:
                @display("p=109.76,553.504,row,100");
        }
        app[numApps]: <> like IApp {
            @display("p=346.69598,74.895996,row,140");
        }
        aid: <aidType> like IAidLayer {
            @display("p=407.68,205.408");
        }
        udp: <default(firstAvailableOrEmpty("Udp"))> like IUdp if hasUdp {
            parameters:
                @display("p=346.69598,370.856");
        }
        tcp: <default(firstAvailableOrEmpty("Tcp", "TcpLwip", "TcpNsc"))> like ITcp if hasTcp {
            parameters:
                @display("p=473.53598,370.856");
        }
        sctp: <default(firstAvailableOrEmpty("Sctp"))> like ISctp if hasSctp {
            parameters:
                @display("p=710.304,370.856");
        }
        ipv4: <default("Ipv4NetworkLayer")> like INetworkLayer if hasIpv4 {
            parameters:
                @display("p=425,507;q=queue");
                //set HostAutoConfigurator (deactivated)
                //use standalone configurator
                configurator.typename = "HostAutoConfigurator";
                *.configurator.interfaces = "wlan";
                *.limitedBroadcast = true;
        }
        ipv6: <default("Ipv6NetworkLayer")> like INetworkLayer if hasIpv6 {
            parameters:
                @display("p=681.312,526.688;q=queue");
        }
        lteNic: <nicType> like ILteNic {
            nodeType = nodeType;
            //d2dCapable = d2dCapable;
            @display("p=549.64,674.06396");
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
        // message dispatcher for SAP between transport and network layer
        tn: MessageDispatcher {
            parameters:
                @display("p=550,446;b=600,5,,,,1");
        }
        // message dispatcher for SAP to link layer
        nl: MessageDispatcher {
            parameters:
                @display("p=550,596;b=600,5,,,,1");
        }
        configurator: HostAutoConfigurator {
            @display("p=170.912,101.92");
        }

    connections allowunconnected:
        //# LTE Nic 
        lteNic.radioIn <-- radioIn;

        lteNic.upperLayerOut --> nl.in++;
        lteNic.upperLayerIn <-- nl.out++;

        radioDriver.upperLayer <--> vanetza.radioDriverData;
        radioDriver.properties --> vanetza.radioDriverProperties;

        //# 
        //# Internal Tcp/Udp applications connections with transport layer
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

        //# 
		//# Internal transport layer connections to network layer
		//# 

        udp.ipOut --> tn.in++ if hasUdp;
        udp.ipIn <-- tn.out++ if hasUdp;

        tcp.ipOut --> tn.in++ if hasTcp;
        tcp.ipIn <-- tn.out++ if hasTcp;

        sctp.ipOut --> tn.in++ if hasSctp;
        tn.out++ --> sctp.ipIn if hasSctp;

        ipv4.ifIn <-- nl.out++ if hasIpv4;
        ipv4.ifOut --> nl.in++ if hasIpv4;

        radioDriver.lowerLayerIn <-- nl.out++ if hasGeoNet;
        radioDriver.lowerLayerOut --> nl.in++ if hasGeoNet;

        ipv4.transportIn <-- tn.out++ if hasIpv4;
        ipv4.transportOut --> tn.in++ if hasIpv4;

        ipv6.ifIn <-- nl.out++ if hasIpv6;
        ipv6.ifOut --> nl.in++ if hasIpv6;

        ipv6.transportIn <-- tn.out++ if hasIpv6;
        ipv6.transportOut --> tn.in++ if hasIpv6;

        tn.out++ --> nl.in++;
        tn.in++ <-- nl.out++;

        at.out++ --> tn.in++;
        at.in++ <-- tn.out++;

        aa.out++ --> at.in++;
        aa.in++ <-- at.out++;
}
