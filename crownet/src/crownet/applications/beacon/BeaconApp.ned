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

package crownet.applications.beacon;
import crownet.applications.common.BaseApp;
import inet.applications.contract.IApp;


moduleinterface IBeaconApp extends IApp {
    parameters:
}

simple BeaconSimple extends BaseApp like IBeaconApp {
    
    parameters:
        @class(crownet::BeaconSimple);       
        // Aid app override defaults
        packetName = default("BeaconSimple");
        mainInterval = default(1.0s);
        mainIntervalJitter = default(0s);
		messageLength = default(300B);
		allEmptyDestAddress = false;
        
        // App logic
        string mobilityModule;
        string neighborhoodTableMobdule;
}

simple BeaconDynamic extends BaseApp like IBeaconApp {
    
    parameters:
        @class(crownet::BeaconDynamic);       
        // Aid app override defaults
        packetName = default("BeaconDynamic");
        mainInterval = default(1.0s);
        mainIntervalJitter = default(0s);
		messageLength = default(300B);
		allEmptyDestAddress = false;
        
        // App logic
        string mobilityModule;
        string neighborhoodTableMobdule;
}

