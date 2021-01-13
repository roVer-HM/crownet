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

package rover.artery.lte;

import rover.artery.IDispatchRadioDriver;
import artery.nic.IRadioDriver;

simple LteRadioDriver like IDispatchRadioDriver
{
 	parameters:
 	    @class(rover::LteRadioDriver);
 	    @signal[ChannelLoad](type=double);
 	    string dispatchInterfaceName = default("cellular"); //todo make this configuratabel
 	    string interfaceTableModule;
 	    
    gates:
   		inout upperLayer;
		input lowerLayerIn @labels(Ieee802Ctrl/up);
		output lowerLayerOut @labels(Ieee802Ctrl/down);
		output properties;
		
}