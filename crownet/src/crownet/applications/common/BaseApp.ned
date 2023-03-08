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
import crownet.queueing.CrownetActivePacketSourceBase;


moduleinterface IAppConfig {
    gates:
    	input configIn;
}

moduleinterface IBaseApp {
    gates:
        output out;
        input in;
        input scheduleIn;
}

simple BaseApp extends CrownetActivePacketSourceBase like IBaseApp
{
  parameters:
        @class(crownet::BaseApp);
        @display("i=block/app");
        displayStringTextFormat = default("");
        ///* socket module (should be a sibling model)
        string socketModule = default("^.socket");
        string schedulerModule = default("^.scheduler");
        object localInfo = default(crownet::AppInfoLocal{});
        
        // assume BaseApp is part of some crownet app
        string mobilityModule = default("^.^.mobility");
        
        // Application runtime settings
        ///* Start time of application at which message will be received. 
        ///* Negative values raises cRuntimeError.
        double startTime @unit(s) = default(uniform(0s,0.02s));
        ///* Time after which no packet will be send or received.
        ///* stopTime = 0.0s means app will run forever
        double stopTime @unit(s) = default(0.0s); 
        // Max size of application packet containing application headers
        int maxPduLength @unit(b) = default(1400B);
        // Minimum size of application packet. 
        // If -1B application must provide a minmum value programmatically, otherwise an error is thrown
        int minPduLength @unit(b) = default(-1B);  
        

        ///* Socket: If true app will not throw an error if destiantion address is empty.
        ///* Set to true if app only passivly receives messages or has other means 
        ///* to find a valid destination address.
        bool allEmptyDestAddress = default(true);
        
        

       
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation

		// send stats
        @signal[packetSent](type=inet::Packet);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[sentDataRate](title="outgoing data rate"; unit=bps; source="throughput(packetSent)"; record=vector; interpolationmode=none);
        
        // received stats
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdDataRate](title="incomming data rate"; unit=bps; source="throughput(packetReceived)"; record=vector; interpolationmode=none);
        @statistic[rcvdPkLifetime](title="received packet lifetime"; source="dataAge(packetReceived)"; unit=s; record=stats,vector; interpolationmode=none);
        @statistic[rcvdPkHostId](title="received host id"; source="rcvdHostId(packetReceived)"; record=vector; interpolationmode=none);
        @statistic[rcvdPktPerSrcJitter](title="received jitter per source"; source="rcvdPerSrcJitter(packetReceived)"; record=vector; interpolationmode=none);
        @statistic[rcvdPktPerSrcAvgSize](title="received average packet bits per source"; unit=b; source="rcvdPerSrcAvgSize(packetReceived)"; record=vector; interpolationmode=none);
        @statistic[rcvdPktPerSrcCount](title="received communaltive packet count per source"; unit=b; source="rcvdPerSrcCount(packetReceived)"; record=vector; interpolationmode=none);
        @statistic[rcvdPerSrcTotalCount](title="total count of packets send by source"; source="rcvdPerSrcTotalCount(packetReceived)"; record=vector; interpolationmode=none);
        @statistic[rcvdPktPerSrcLossCount](title="received communaltive packet loss count per source"; unit=b; source="rcvdPerSrcLossCount(packetReceived)"; record=vector; interpolationmode=none);
		@statistic[rcvdPktPerSrcSeqNo](title="received packet sequence number per source"; source="rcvdSequenceId(packetReceived)"; record=vector; interpolationmode=none);
        @statistic[rcvdPktAvgSize](title="received average packet bits"; unit=b; source="rcvdAvgSize(packetReceived)"; record=vector; interpolationmode=none);
        @statistic[rcvdPktCount](title="received communaltive packet count"; unit=b; source="rcvdCount(packetReceived)"; record=vector; interpolationmode=none);
        @statistic[rcvdSrcCount](title="number of different sources received"; unit=int; source="rcvdSrcCount(packetReceived)"; record=vector; interpolationmode=none);

        
        // queueing statitics
        @signal[packetPulled](type=inet::Packet);
        @signal[packetCreated](type=inet::Packet);
        @statistic[packetPulled](title="packets pulled"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetCreated](title="packets created"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
	 gates:
   		input in;
        input scheduleIn;

}
