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

package crownet.artery.lte;

import crownet.artery.IDispatchRadioDriver;
import artery.nic.IRadioDriver;

simple LteRadioDriver like IDispatchRadioDriver
{
 	parameters:
 	    @class(crownet::LteRadioDriver);
 	    @signal[ChannelLoad](type=double);
 	    string dispatchInterfaceName = default("cellular"); //todo make this configuratabel
 	    string interfaceTableModule;
 	    // CCH=180 (default) see  TS 102 965 V1.3.1 Annex A,
 	    // the XmlMultiChannelPolicy class and the Middleware.ned parameter mcoPolicy
 	    // [SCH0 = CCH,  SCH1 = 176, SCH2 = 178, SCH3 = 174]
 	    string channelNumber = default("CCH");

    gates:
   		inout upperLayer;
		input lowerLayerIn @labels(Ieee802Ctrl/up);
		output lowerLayerOut @labels(Ieee802Ctrl/down);
		output properties;

}
