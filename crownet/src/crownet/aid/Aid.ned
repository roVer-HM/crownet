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

package crownet.aid;

moduleinterface IAid
{
    parameters:
        @display("i=block/app");
    gates:
        input upperIn;
        output upperOut;
        input lowerIn;
        output lowerOut;
}

simple Aid like IAid {
    parameters:
    	@display("i=block/app");
    	@class(crownet::Aid);
    	double stopOperationExtraTime @unit(s) = default(0s);    // extra time after lifecycle stop operation finished
    	double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
    	@signal[packetReceivedFromUpper](type=cPacket);
    	@signal[packetReceivedFromLower](type=cPacket);

    	//Default Udp Socket Options
    	int timeToLive = default(-1); // if not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        int dscp = default(-1); // if not -1, set the DSCP field (on IPv4/IPv6) of sent packets to this value
        int tos = default(-1); // if not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value
        string multicastInterface = default("");  // if not empty, set the multicast output interface option on the socket (interface name expected)
        bool receiveBroadcast = default(true); // if true, makes the socket receive broadcast packets
        bool joinLocalMulticastGroups = default(true); // if true, makes the socket receive packets from all multicast groups set on local interfaces
	    //todo: FixMe. The Aid layer should be able to determine this dynamically ...
	    string interfaceTableModule; // The path to the InterfaceTable module


    gates:
    	input upperIn;
    	output upperOut;
    	input lowerIn;
    	output lowerOut;
}
