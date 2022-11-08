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

package crownet.mec;

import inet.applications.contract.IApp;

simple UEMECApp like IApp
{
    parameters:
        @display("i=block/source"); 
        
        int localPortMEC = default(4000);
        int deviceAppPort = default(4000);
        string deviceAppAddress;
        
        int localPortD2D = default(5000);
        
        string interfaceTableModule;
        string multicastInterface = default("cellular");
        
        string destAddressD2D;
        
        string personNode;
        string vehicleNode;

        string mecAppName = default("MECBeaconApp");
        bool disseminateMECBeacons = default(false);
        bool disseminateD2DBeacons = default(false);
        int beaconType = default(0);
        
        double period @unit("s") = default(1s);
        double evalPeriod @unit("s") = default(0.1s);
        double startTime @unit("s") = default(0.5s); 
        double stopTime @unit("s") = default(120s);
        
    gates:
        output socketOut;
        input socketIn;
                
}
