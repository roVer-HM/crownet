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

package crownet.artery.traci;

import traci.NodeManager;

simple VadereNodeManager like NodeManager{

parameters:
    @class(crownet::VadereNodeManager);
    @signal[traci.node.add](type=string);
    @signal[traci.node.update](type=unsigned long);
    @signal[traci.node.remove](type=string);
    @signal[traci.person.add](type=string);
    @signal[traci.person.update](type=string);
    @signal[traci.person.remove](type=string);
    string coreModule;
    string mapperModule;
    string objectSinkModule;
    string subscriptionsModule;

}
