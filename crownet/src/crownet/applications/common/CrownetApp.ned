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
import inet.queueing.contract.ITrafficConditioner;
import inet.queueing.contract.IPacketMeter;
import crownet.applications.common.scheduler.IAppScheduler;

module CrownetApp like IApp, IAppConfig
{
    parameters:
        @display("bgb=500,283");
    gates:
        input socketIn;
        output socketOut;
        input configIn;

    submodules:
        app: <> like IBaseApp {
            @display("p=142,208,row,140");
        }

        packetMeterOut: <default("GenericPacketMeter")> like IPacketMeter {

            @display("p=243,85");
        }

        packetMeterIn: <default("GenericPacketMeter")> like IPacketMeter {

            @display("p=243,208");
        }


        socket: <> like ISocketManager {
            @display("p=407,208");
        }

        shaper: <default("EmptyTrafficConditioner")> like ITrafficConditioner {

            @display("p=348,85");
        }
        scheduler: <> like IAppScheduler {

            @display("p=65,64");
        }


    connections:
        app.out --> packetMeterOut.in;
        packetMeterOut.out --> shaper.in;
        shaper.out --> socket.in;

        socket.toApp --> packetMeterIn.in;
        packetMeterIn.out --> app.in;

        scheduler.out --> app.scheduleIn;

        socket.toStack --> { @display("m=s"); } --> socketOut;
        socket.fromStack <-- { @display("m=s"); } <-- socketIn;

        configIn --> { @display("m=w"); } --> scheduler.schedulerIn;
}



module CrownetUdpApp extends CrownetApp
{
    parameters:
        socket.typename = default("UdpSocketManager");
}


module CrownetAidApp extends CrownetApp
{
    parameters:
        socket.typename = default("AidSocketManager");

}


module BeaconApp extends CrownetApp
{
    parameters:
        @BeaconApp;
        @display("i=block/broadcast;is=s");
        app.typename = default("BeaconDynamic");
        socket.typename = default("UdpSocketManager");
        socket.destPort = default(30001);
        socket.localPort = default(30001);
        packetMeterIn.typename = "ApplicationPacketMeterIn";
        packetMeterIn.appInfoClass = default("crownet::BeaconReceptionInfo");
}


module DensityMapApp extends CrownetApp
{
    parameters:
        @MapApp;
        @display("i=block/users;is=s");
        app.typename = default("DensityMapAppSimple");
        socket.typename = default("UdpSocketManager");
        socket.destPort = default(30002);
        socket.localPort = default(30002);
}

