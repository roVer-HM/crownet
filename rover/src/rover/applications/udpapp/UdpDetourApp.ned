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
	    double incidentTime @unit(s) = default(-1s); // time at which the incident (see reason) is triggerd. If -1 no incident is triggerd and app only performs passive task (receive and repeate)
	    string reason = default("Closed Entrance");
	    int closedTarget = default(-1);		 // 42
	    string alternativeRoute = default(""); // 1,3,4 (replace 42 with 1->3->4 
	    double repeatTime @unit(s)= default(60.0s); // time how long a received message will be hold and repated until it discarded.
	    volatile double repeateInterval @unit(s) = default(10.0s); // repeate interval to resend received messages util timeToLive is reached.
}