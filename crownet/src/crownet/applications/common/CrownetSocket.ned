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

package crownet.applications.common;

import inet.queueing.contract.IPassivePacketSink;

moduleinterface ISocketManager extends IPassivePacketSink
{
    parameters:
         @display("i=block/app");
         int localPort;
         int destPort;
    gates:
        output toApp;
        input fromStack;
        output toStack;
}

simple BaseSocketManager like ISocketManager {
    parameters:
        @class("crownet::BaseSocketManager");
        
        // Base addressing paramters
        string localAddress = default("");
        int localPort = default(-1);  // local port (-1: use ephemeral port)
        string destAddresses = default("224.0.0.1"); // ALL_HOST_MCAST (automatically joined)
        int destPort; //
        bool dontFragment = default(false); // if true, asks IP to not fragment the message during routing
        
        string appModule = "^.app";
        
    gates:
        input in @labels(push); // fromApp
        output toApp;
        input fromStack;
        output toStack;
}

simple UdpSocketManager extends BaseSocketManager {
    parameters:
        @class("crownet::UdpSocketManager");
        string interfaceTableModule; // The path to the InterfaceTable module
        int timeToLive = default(-1); // if not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        int dscp = default(-1); // if not -1, set the DSCP field (on IPv4/IPv6) of sent packets to this value
        int tos = default(-1); // if not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value
        string multicastInterface = default("");  // if not empty, set the multicast output interface option on the socket (interface name expected)
        bool receiveBroadcast = default(true); // if true, makes the socket receive broadcast packets
        bool joinLocalMulticastGroups = default(true); // if true, makes the socket receive packets from all multicast groups set on local interfaces
        
}

simple AidSocketManager extends BaseSocketManager {
    parameters:
        @class("crownet::AidSocketManager");
        double minRate = default(1.0);
        double maxRate = default(2.0);
}
