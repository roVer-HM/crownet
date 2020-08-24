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

#include "rover/common/OsgCoordianteTransformer.h"

using namespace omnetpp;
using namespace inet;

namespace rover {

OsgCoordianteTransformer::OsgCoordianteTransformer(std::string epgs_code,
                                                   inet::Coord offset)
    : epsg_code(epgs_code),
      offset(offset),
      c_srs(osgEarth::SpatialReference::get(epgs_code)) {}

OsgCoordianteTransformer::~OsgCoordianteTransformer() {}

inet::Coord OsgCoordianteTransformer::convert2D(inet::GeoCoord& c) const {
  return convert2D(c.latitude.get(), c.longitude.get());
}

inet::Coord OsgCoordianteTransformer::convert2D(double lat, double lon) const {
  osgEarth::GeoPoint input{c_srs->getGeographicSRS(), lon,
                           lat};  // order!! lon first
  osgEarth::GeoPoint output = input.transform(c_srs);
  if (output.isValid()) {
    return inet::Coord(output.x() - offset.x, output.y() - offset.y,
                       output.z() - offset.z);
  } else {
    throw cRuntimeError("invalid transformation");
  }
}

inet::GeoCoord OsgCoordianteTransformer::convertGeo(inet::Coord& c) const {
  return convertGeo(c.x, c.y, c.z);
}

inet::GeoCoord OsgCoordianteTransformer::convertGeo(double x, double y,
                                                    double z) const {
  osgEarth::GeoPoint input{c_srs, x + offset.x, y + offset.y, z + offset.z};
  osgEarth::GeoPoint output = input.transform(c_srs->getGeographicSRS());
  // output (lon, lat, alt);
  if (output.isValid()) {
    // inet::GeoCoord order (lat, lon, alt)
    return inet::GeoCoord(deg(output.y()), deg(output.x()), m(output.z()));
  } else {
    throw cRuntimeError("invalid transformation");
  }
}

}  // namespace rover
