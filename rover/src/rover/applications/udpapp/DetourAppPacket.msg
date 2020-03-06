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

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.applications.base.ApplicationPacket;
import inet.common.geometry.Geometry;
import inet.common.TagBase;


namespace rover;


enum DetourPktType {
	INCIDENT = 0;
	PROPAGATE = 1;
}

class CurrentLocationTag extends inet::TagBase
{
    inet::Coord location;
}

class DetourAppPacket extends inet::ApplicationPacket
{
 	   string incidentReason;
 	   simtime_t repeatTime;
 	   simtime_t repeateInterval = 20.0;
 	   string closedTarget;
 	   string alternativeRoute[];
 	   simtime_t firstHopTime;
 	   uint32_t firstHopId;
 	   inet::Coord firstHopOrigin;
 	   simtime_t lastHopTime;
 	   uint32_t lastHopId;
 	   inet::Coord lastHopOrigin;
 	   DetourPktType type;
}