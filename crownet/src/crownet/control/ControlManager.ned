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

package crownet.control;

//
// TODO auto-generated module
//
moduleinterface IControlManager {

}

simple ControlManager like IControlManager
{
    parameters:
        @class(crownet::ControlManager);
        string host;
        int port;
        string coreModule;
        // todo: (CM) must be set to emtpy string to allow simulations without specifiying a 'sensor'
        string globalDcdModule = default("");
        double startTime  @unit(s) = default(0.0s);
        string controlGate = default("configIn");
}
