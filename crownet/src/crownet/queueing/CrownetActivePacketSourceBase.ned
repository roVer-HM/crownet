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

package crownet.queueing;

import inet.queueing.base.ActivePacketSourceBase;

simple CrownetActivePacketSourceBase extends ActivePacketSourceBase
{
    parameters:
        string packetName = default("packet");
        packetNameFormat = default("%N-%i-%c"); // %N packetName %n module name, %c packet number, %l packet length, %d packet data, %t simulation time, %e event number
        displayStringTextFormat = default("created %p pk (%l)");
        packetLength = default(-1b); // packet length is defined programmatically by application.
        packetRepresentation = default("applicationPacket");
        @class(CrownetActivePacketSourceBase);
        @display("i=block/source");
    gates:
        output out @labels(push);
}