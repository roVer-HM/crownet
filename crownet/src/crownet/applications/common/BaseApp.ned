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

import inet.applications.contract.IApp;

simple BaseApp like IApp
{
  parameters:
        @class(crownet::BaseApp);
        @display("i=block/app");
        // Application runtime settings
        double startTime @unit(s) = default(0.0s); // start first packet, afterwarts time + sendIntervall. Negative values raises cRuntimeError.
        double stopTime @unit(s) = default(0.0s); // time of finishing sending, 0.0s means forever
        volatile double appMainInterval @unit(s) = default(-1s); // intervall to enter fsmAppMain to process app logic.
        										  // if set to -1 fsmAppMain will not be used and user must provide own
        										  // handling. fsmAppMain does nothing on default.
        volatile double appMainIntervalJitter @unit(s) = default(0s); // 
        volatile int messageLength @unit(B);
        // Base addressing paramters
        string localAddress = default("");
        int localPort = default(-1);  // local port (-1: use ephemeral port)
        string destAddresses = default("224.0.0.1"); // ALL_HOST_MCAST (automatically joined)
        int destPort; //
        string packetName = default("BasicAppData");
        // IP Setting
        bool dontFragment = default(false); // if true, asks IP to not fragment the message during routing
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
        // signals
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[throughput](title="throughput"; unit=bps; source="throughput(packetReceived)"; record=vector);
        @statistic[rcvdPkLifetime](title="received packet lifetime"; source="dataAge(packetReceived)"; unit=s; record=stats,vector; interpolationmode=none);
        @statistic[rcvdPkSeqNo](title="received packet sequence number"; source="appPkSeqNo(packetReceived)"; record=vector; interpolationmode=none);

	 gates:
   		input socketIn;
        output socketOut;
}
