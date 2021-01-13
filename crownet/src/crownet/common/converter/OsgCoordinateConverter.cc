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

#include "crownet/common/converter/OsgCoordinateConverter.h"

#include <vector>

using namespace omnetpp;
using namespace inet;

namespace crownet {

OsgCoordinateConverter::OsgCoordinateConverter(inet::Coord zBound,
                                               inet::Coord _simBound,
                                               std::string epgs_code) {
  zoneOriginOffset.x = zBound.x;
  zoneOriginOffset.y = zBound.y;
  zoneOriginOffset.z = zBound.z;

  std::vector<traci::TraCIPosition> vec;
  traci::TraCIPosition upperRight;
  upperRight.x = _simBound.x;
  upperRight.y = _simBound.y;
  vec.push_back(traci::TraCIPosition{});
  vec.push_back(upperRight);
  simBound = traci::Boundary(vec);

  epgs_code = epgs_code;
}

double OsgCoordinateConverter::getBoundaryWidth() const {
  return std::abs(simBound.upperRightPosition().x -
                  simBound.lowerLeftPosition().x);
}
double OsgCoordinateConverter::getBoundaryHeight() const {
  return std::abs(simBound.upperRightPosition().y -
                  simBound.lowerLeftPosition().y);
}

OsgCoordinateConverter::OsgCoordinateConverter(
    traci::TraCIPosition zoneOriginOffset, traci::Boundary simBound,
    std::string epgs_code)
    : zoneOriginOffset(zoneOriginOffset),
      simBound(simBound),
      epsg_code(epgs_code),
      c_srs(osgEarth::SpatialReference::get(epgs_code)) {}

OsgCoordinateConverter::~OsgCoordinateConverter() {}

osgEarth::GeoPoint OsgCoordinateConverter::addZoneOriginOffset(
    const traci::TraCIPosition& pos) const {
  return osgEarth::GeoPoint{c_srs, pos.x + zoneOriginOffset.x,
                            pos.y + zoneOriginOffset.y,
                            pos.z + zoneOriginOffset.z};
}

void OsgCoordinateConverter::removeZoneOriginOffset(
    osgEarth::GeoPoint& pos) const {
  pos.x() -= zoneOriginOffset.x;
  pos.y() -= zoneOriginOffset.y;
  pos.z() -= zoneOriginOffset.z;
}

inet::Coord OsgCoordinateConverter::convert2D(inet::GeoCoord& c) const {
  return convert2D(c.latitude.get(), c.longitude.get());
}

inet::Coord OsgCoordinateConverter::convert2D(double lat, double lon) const {
  osgEarth::GeoPoint input{c_srs->getGeographicSRS(), lon,
                           lat};  // order!! lon first
  osgEarth::GeoPoint output = input.transform(c_srs);
  if (output.isValid()) {
    return inet::Coord(output.x() - zoneOriginOffset.x,
                       output.y() - zoneOriginOffset.y,
                       output.z() - zoneOriginOffset.z);
  } else {
    throw cRuntimeError("invalid transformation");
  }
}

}  // namespace crownet
