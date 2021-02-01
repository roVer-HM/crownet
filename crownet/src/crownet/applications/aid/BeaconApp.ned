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

package crownet.applications.aid;
import crownet.applications.common.AidBaseApp;
import inet.applications.contract.IApp;


moduleinterface IBeaconApp extends IApp {
    parameters:
}

simple BeaconSimple extends AidBaseApp like IBeaconApp {
    
    parameters:
        @class(crownet::BeaconSimple);       
        // Aid app override defaults
        packetName = default("BeaconSimple");
        appMainInterval = default(1.0s);
        appMainIntervalJitter = default(0s);
        
        // Aid app fix defaults
        destPort = 30002;
        localPort = 30002;
        
        // App logic
        string mobilityModule;
        string neighborhoodTableMobdule;
        
}