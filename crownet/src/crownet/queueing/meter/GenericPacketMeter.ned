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

package crownet.queueing.meter;

import inet.queueing.base.PacketMeterBase;
import inet.queueing.contract.IPacketMeter;
import inet.queueing.meter.ExponentialRateMeter;


simple GenericPacketMeter extends PacketMeterBase like IPacketMeter 
{
 	parameters:
 	    @display("i=block/star");
        @class(crownet::GenericPacketMeter);
        // the statistical value is the data rate of the incoming packets
        @statistic[incomingDataRateXX](title="incoming data rateXX"; interval=10s; source=throughput(merge(packetPushedIn, packetPulledIn)); record=vector; unit=bps; interpolationmode=none);
        
        
        // %p packet count; %l bit count;  %r packet rate; %R data rate
        displayStringTextFormat = default("%r | %R");
        object meter = default(crownet::EmaPacketMeter{alpha: 1/16});
//		object meter = default(crownet::TemaPacketMeter{beta: 15/16});
        bool markPackets = default(false);
          
}

simple ApplicationPacketMeterIn extends GenericPacketMeter
{
    parameters:
        @display("i=block/star");
        @class(crownet::ApplicationPacketMeterIn);
        
        // class name of application info object that handles 
        // application-source level statistics
        string appInfoClass;
        // add stat results to packet for application internal processing
        bool appendAppInfoTag = default(true);
        // average computation based on RTPC (RFC 3350 page 31, 40)
       	double ema_smoothing_jitter = default(1/16);
		double ema_smoothing_packet_size = default(1/16);
        
        
}