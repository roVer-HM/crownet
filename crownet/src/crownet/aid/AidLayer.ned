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

package crownet.aid;
import crownet.aid.IAid;
import crownet.applications.aid.IDensityMapApp;
import crownet.applications.aid.IBeaconApp;
import inet.common.MessageDispatcher;

moduleinterface IAidLayer
{
    gates:
        input upperIn;
        output upperOut;
        input lowerIn;
        output lowerOut;
}

module SimpleAidLayer like IAidLayer
{
    parameters:
        @display("i=block/app");

    gates:
        input upperIn;
        output upperOut;
        input lowerIn;
        output lowerOut;

    submodules:
        aid: <default("Aid")> like IAid {
            parameters:
                @display("p=218,121;is=s");
        }
        densityMapApp: <default("ArteryDensityMapApp")> like IDensityMapApp if typename!="" {
            @display("p=413,121;i=block/users;is=s");
        }

        beaconApp: <default("BeaconSimple")> like IBeaconApp if typename!="" {
            @display("p=532,121;i=block/broadcast;is=s");
        }

        // Message Dispatcher
        upper: MessageDispatcher {
            parameters:
                @display("p=351,49;b=600,5");
        }
        lower: MessageDispatcher {
            parameters:
                @display("p=351,186;b=600,5");
        }
    connections allowunconnected:
        upperIn --> upper.in++;
        upperOut <-- upper.out++;

        lower.out++ --> lowerOut;
        lower.in++ <-- lowerIn;

        upper.out++ --> aid.upperIn;
        upper.in++ <-- aid.upperOut;

        aid.lowerOut --> lower.in++;
        aid.lowerIn <-- lower.out++;

        densityMapApp.socketIn <-- upper.out++ if  exists(densityMapApp);
        densityMapApp.socketOut --> upper.in++ if exists(densityMapApp);

        beaconApp.socketIn <-- upper.out++ if exists(beaconApp);
        beaconApp.socketOut --> upper.in++ if exists(beaconApp);

}
