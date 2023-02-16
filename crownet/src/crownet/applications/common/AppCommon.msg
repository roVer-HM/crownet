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
import inet.common.geometry.Geometry;
import inet.common.TagBase;
import inet.common.Units;

cplusplus {{
#include "inet/common/packet/Packet.h"
using namespace inet;
}}

class CurrentLocationTag extends inet::TagBase
{
    inet::Coord location;
}

class SequenceIdTag extends inet::TagBase
{
	long sequenceNumber;    
}

class HostIdTag extends inet::TagBase
{
	int hostId;    
}

class HopCount extends inet::TagBase
{
 	int hops;   
}


class BurstTag extends inet::TagBase
{
    simtime_t burstCreationTime;
    long burstIndex;
    int burstPktCount;
    inet::b burstSize;
    bool skipStatsProcessing = false;
}
class BurstSkipTag extends inet::TagBase{}
