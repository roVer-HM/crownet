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

package crownet.applications.common;


import inet.applications.contract.IApp;
import crownet.applications.common.ISocketManager;
import crownet.applications.common.IBaseApp;



module CrownetApp like IBaseApp
{
    parameters:
        @display("bgb=500,225");
    gates:
        output socketOut;
        input socketIn;
        input configIn;

    submodules:
        app: <> like IBaseApp {
            @display("p=95,115,row,140");
        }

        socket: <> like ISocketManager {
            @display("p=255,115");
        }

    connections:
        app.socketOut --> socket.fromApp;
        app.socketIn <-- socket.toApp;

        socket.toStack --> socketOut;
        socket.fromStack <-- socketIn;
        
        app.configIn <-- configIn;
}

module CrownetUdpApp extends CrownetApp {
    parameters:
        socket.typename = default("UdpSocketManager");   
}

module CrownetAidApp extends CrownetApp {
    parameters:
        socket.typename = default("AidSocketManager");   
}


