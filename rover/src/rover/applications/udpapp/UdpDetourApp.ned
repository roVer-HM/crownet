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

package rover.applications.udpapp;

import inet.applications.udpapp.UdpBasicApp;

simple UdpDetourApp extends UdpBasicApp
{
	parameters:
	    @class(rover::UdpDetourApp);
	    // sender parameters
	    double incidentTime @unit(s) = default(-1.0s); // time at which the incident (see reason) is triggerd. If -1 no incident is triggerd and app only performs passive task (receive and repeate)
	    string reason = default("");		 // unique identifier
	    int closedTarget = default(-1);		 // 42
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
}