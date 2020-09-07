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

package rover.mobility;

import inet.mobility.base.MobilityBase;
import org.car2x.veins.subprojects.veins_inet.vadere.VaderePersonMobility;

simple VaderePersonTracedMobility extends VaderePersonMobility
{
    parameters:
        @class(rover::VaderePersonTracedMobility);
        int ringBufferSize = default(40); // number of position / time pairs to save in 
        								  // nodes for easy application access.
        double recordThreshold @unit(s) = default(100ms); // minimal time delta needed to write new location 
        												  // in ringBuffer.
}
