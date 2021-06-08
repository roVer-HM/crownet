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

package crownet.common;


moduleinterface ICoordConverter {


}

// locally defined converter. Use configuration of parameters
simple OsgCoordConverterLocal like ICoordConverter {
    parameters:
        @class(crownet::OsgCoordConverterLocal);
        string epsg_code = default("EPSG:32632");
        double offset_x = default(0.0);
        double offset_y = default(0.0);
        double xBound;
        double yBound;
}


simple OsgCoordConverterVadere like ICoordConverter {
    parameters:
        @class(crownet::OsgCoordConverterVadere);
        string coreModule = default("traci.core");
}

simple OsgCoordConverterSumo like ICoordConverter {
    parameters:
        @class(crownet::OsgCoordConverterSumo);
        string coreModule = default("traci.core");
        string epsg_code = default("EPSG:32632");
}

