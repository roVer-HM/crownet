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

package rover.aid;
import rover.aid.IAid;
import rover.aid.SimpleAidConfig;
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
        aid: <default("SimpleAid")> like IAid {
            parameters:
                @display("p=164,121;is=s");
        }
        aidCfg: <default("SimpleAidConfig")> like IAidConfig {
            @display("p=394,121;is=s");
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

}
