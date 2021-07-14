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

package crownet.emulation.locationExporter;

module NodeLocationExporter
{
    parameters:
        @display("i=block/sink");
        int port;
        string address;
        string interfaceTableModule;
        int offsetEasting = default(0);
        int offsetNorthing = default(0);
        double startTime @unit(s) = default(0s); // application start time
        double stopTime @unit(s) = default(-1s);  // time of finishing receiving
        double interval;
        
        @class(crownet::NodeLocationExporter);
        
    gates:
        input socketInExternal @labels(UdpControlInfo/up);
        output socketOutExternal @labels(UdpControlInfo/down);
    
}