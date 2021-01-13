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
import inet.networklayer.common.IpProtocolId;   //FIXME remove it, unused
import inet.networklayer.common.L3Address;

namespace rover;

//
// AID command codes, sent by the application to AID. These constants
// should be set as message kind on a message sent to the AID entity.
//
enum AidCommandCode
{
	AID_C_BIND = 1;  // send by application to initiliaze AID Socket.
	AID_C_APP_REQ = 2; // send by application requirements to AID Layer. (used to determine matching dissemination)
	AID_C_APP_CAP = 3; // send by application capabilities to AID Layer. (used to determine matching dissemination)
	AID_C_CONNECT = 4; // send by application to finalize setup.
	AID_C_DATA = 5; // send()/sendTo() by application
	AID_C_CLOSE = 6; //
	AID_C_DESTROY = 7; // destroy(); see ~AidDestroyCommand
}

enum AidRecipientClass {
	ALL = 1;		// messages are for all recipient indepenten of geograpic location
	ALL_LOCAL=2;
	SOME=3;
	SOME_LOCAL=4;
	ONE=5;
	ONE_LOCAL=6;
}



//
// AID indications, sent by AID to the application. AID will set these
// constants as message kind on messages it sends to the application.
//
enum AidStatusInd
{
    AID_I_DATA = 0;
    AID_I_ERROR = 1;
    AID_I_STATUS = 2;
    AID_I_CLOSE = 3;
    
}

class AidCommand extends cObject
{
    int userId = -1;   // id than can be freely used by the app
}

class AidBindCommand extends AidCommand {
	inet::L3Address localAddr;
	int localPort = -1;
}

//
// Base class for AID socket option control info classes.
//
class AidAppReqCommand extends AidCommand {
	double minRate; // minimum neede message rate of the application
	double maxRate = -1.0; // maximum usefull rate for the application (-1.0 no max rate)
	AidRecipientClass recipientClass;
	inet::L3Address remoteAddr;
	int remotePort = -1;
}

class AidAppCapCommand extends AidCommand {
	// todo define capabilities
}

class AidConnectCommand extends AidCommand {
	// todo define capabilities
}

class AidCloseCommand extends AidCommand {

}

class AidDestroyCommand extends AidCommand {

}

class AidControlInfo extends AidCommand {

}

class AidErrorInfo extends AidControlInfo
{
    
}


class AidStatusInfo extends AidControlInfo {
	int statusCode = -1;
}



