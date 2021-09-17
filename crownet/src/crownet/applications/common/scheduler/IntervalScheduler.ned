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

import crownet.applications.common.scheduler.AppSchedulerBase;


simple IntervalScheduler extends AppSchedulerBase {
    parameters:
    	volatile double generationInterval @unit(s); // time interval between two subsequent token generation events
		volatile int numberPackets = default(1); // if < 0 ignore and use amoutOfData
		volatile double amoutOfData @unit(B) = default(-1B); // if numberPackets is -1 this must be set.
		
		int maxNumberPackets = default(-1); // -1 unlmited number of packets
		double maxData @unit(B) = default(-1B); // -1 unlimited number of data 
    	@class(crownet::IntervalScheduler);
}
