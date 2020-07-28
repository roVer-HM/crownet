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

package rover.applications.common;

import inet.applications.contract.IApp;

simple Foo {}

simple BaseApp like IApp
{
  parameters:
        @class(rover::BaseApp);
        @display("i=block/app");
        // Application runtime settings
        double startTime @unit(s) = default(0.0s); // start first packet, afterwarts time + sendIntervall. Negative values raises cRuntimeError.
        double stopTime @unit(s) = default(-1s); // time of finishing sending, -1s means forever
        volatile double appMainInterval @unit(s) = default(-1s); // intervall to enter fsmAppMain to process app logic. 
        										  // if set to -1 fsmAppMain will not be used and user must provide own 
        										  // handling. fsmAppMain does nothing on default.
        // Base addressing paramters
        string localAddress = default("");
        int localPort = default(-1);  // local port (-1: use ephemeral port)
        string destAddresses = default(""); // list of IP addresses, separated by spaces ("": don't send)
        int destPort; // 
        string interfaceTableModule; // The path to the InterfaceTable module
        string packetName = default("BasicAppData");
        volatile int messageLength @unit(B); // length of messages to generate, in bytes
        // IP Setting
        bool dontFragment = default(false); // if true, asks IP to not fragment the message during routing       
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
        // signals
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[throughput](title="throughput"; unit=bps; source="throughput(packetReceived)"; record=vector);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPkLifetime](title="received packet lifetime"; source="dataAge(packetReceived)"; unit=s; record=stats,vector; interpolationmode=none);
        @statistic[rcvdPkSeqNo](title="received packet sequence number"; source="appPkSeqNo(packetReceived)"; record=vector; interpolationmode=none);
       
	 gates:
   		input socketIn;
        output socketOut;
}

simple AidBaseApp extends BaseApp {
      parameters:
        @class(rover::AidBaseApp);
        @display("i=block/app");
}

simple UdpBaseApp extends BaseApp {
      parameters:
        @class(rover::UdpBaseApp);
        @display("i=block/app");
        // udp socket options
        int timeToLive = default(-1); // if not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        int dscp = default(-1); // if not -1, set the DSCP field (on IPv4/IPv6) of sent packets to this value
        int tos = default(-1); // if not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value
        string multicastInterface = default("");  // if not empty, set the multicast output interface option on the socket (interface name expected)
        bool receiveBroadcast = default(true); // if true, makes the socket receive broadcast packets
        bool joinLocalMulticastGroups = default(false); // if true, makes the socket receive packets from all multicast groups set on local interfaces		
}


