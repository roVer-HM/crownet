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

package rover.applications.udpapp.detour;

import inet.applications.contract.IApp;
import rover.applications.common.UdpBaseApp;

simple UdpDetourApp like IApp
{
	parameters:
	    @class(rover::UdpDetourApp);
	    @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @signal[detourIncident](type=inet::Packet);
	    @statistic[incidentAge](title="incidentAge"; source="incidentAge(packetReceived)"; unit=s; record=vector,stats; interpolationmode=none);
	    @statistic[lastHopAge](title="lastHopAge"; source="lastHopAge(packetReceived)"; unit=s; record=vector,stats; interpolationmode=none);
	    @statistic[incidentReceived](title="incidentReceived"; source="packetReceived"; record=incident; interpolationmode=none);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[throughput](title="throughput"; unit=bps; source="throughput(packetReceived)"; record=vector);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @lifecycleSupport;
	    // sender parameters
	    double incidentTime @unit(s) = default(-1.0s); // time at which the incident (see reason) is triggerd. If -1 no incident is triggerd and app only performs passive task (receive and repeate)
	    string reason = default("");		 // unique identifier
	    string closedTarget = default("-1");		 // 42
	    string alternativeRoute = default(""); // comma seperated list (no spaces!)
	    // receiver|source parameters
	    
	    // Timeinterval in which the message should be retransmitted to keep the information alive.
	    // As sender: Value must be greate than 0! This is transmitted in the packet and will be followed by the receiver if not overwritten
	    // As receiver: -1.0 use information in received packet, otherwise use value set here.
	    double repeatTime @unit(s)= default(-1.0s); // As sender: must be set if -1.0 use information in received message
	    // Time between retransmission. Number of retransmission are calculated as repeatTime / repeteInterval
	    // As sender: Value must be greate than 0! This is transmitted in the packet and will be followed by the receiver if not overwritten
	    // As receiver: -1.0 use information in received packet, otherwise use value set here.
	    volatile double repeateInterval @unit(s) = default(-1.0s); // if -1.0 use information in received message.
	    bool notifyMobilityProvider = default(false);
	    string interfaceTableModule;   // The path to the InterfaceTable module
        int localPort = default(-1);  // local port (-1: use ephemeral port)
        string destAddresses = default(""); // list of IP addresses, separated by spaces ("": don't send)
        string localAddress = default("");
        int destPort;
        volatile int messageLength @unit(B); // length of messages to generate, in bytes
        double startTime @unit(s) = default(0s); // application start time. -1s means app is deactivated.
        double stopTime @unit(s) = default(-1s);  // time of finishing sending, -1s means forever
        int timeToLive = default(-1); // if not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        bool dontFragment = default(false); // if true, asks IP to not fragment the message during routing
        int dscp = default(-1); // if not -1, set the DSCP field (on IPv4/IPv6) of sent packets to this value
        int tos = default(-1); // if not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value
        string multicastInterface = default("");  // if not empty, set the multicast output interface option on the socket (interface name expected)
        bool receiveBroadcast = default(true); // if true, makes the socket receive broadcast packets
        bool joinLocalMulticastGroups = default(false); // if true, makes the socket receive packets from all multicast groups set on local interfaces
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation    
	 gates:
   		input socketIn;
        output socketOut;
}

simple UdpDetourAppVadere extends UdpDetourApp
{
	parameters:
	    @class(rover::UdpDetourAppVadere);
	    // sender parameters
	    notifyMobilityProvider = default(true);
	    string mobilityModule = default(".mobility");
}

