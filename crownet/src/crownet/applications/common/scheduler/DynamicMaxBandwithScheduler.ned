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

package crownet.applications.common.scheduler;

import crownet.applications.common.scheduler.IntervalScheduler;

simple DynamicMaxBandwithScheduler extends IntervalScheduler {
    parameters:
      	@class(crownet::DynamicMaxBandwithScheduler);      
		double rndIntervalLowerBound = default(0.5);
		double rndIntervalUpperBound = default(1.5);
		int maxApplicationBandwith @unit(bps) = default(1kbps); // if numberPackets is -1 this must be set.
		int estimatedAvgPaketSize @unit(b) = default(50B);		
		
		string appRxInfoProviderModule = default("^.packetMeterIn");
		
		@signal[txInterval_s](type=simtime_t);
		@statistic[txInterval](title="Transmission interval with random  portion between scheduling calls"; unit=s; source=txInterval_s; record=vector; interpolationmode=none);
    	@signal[txDetInterval_s](type=simtime_t);
		@statistic[txDetInterval](title="Deterministic transmission interval between scheduling calls"; unit=s; source=txInterval_s; record=vector; interpolationmode=none);
}
