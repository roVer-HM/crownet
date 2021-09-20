
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

package crownet.applications.vru;
import crownet.applications.common.BaseApp;
import crownet.applications.common.IBaseApp;

// Marker Interface for Vulnerable Road User app.
moduleinterface IVRU extends IBaseApp {

}


simple VruEtsiSimple extends BaseApp like IVRU {
    parameters:
        @class(crownet::VruEtsiSimple);
	    packetLength = default(1000B); // length of messages to generate, in bytes
	    packetName = default("VRU_AID");
	    double minRate = default(1); // Hz
	    double maxRate = default(10); // Hz
	    string mobilityModule = default("^.mobility");


}

simple VruSimple extends BaseApp like IVRU {
    parameters:
        @class(crownet::VruSimple);
	    packetLength = default(1000B);
	    packetName = default("VRU_AID");
}
