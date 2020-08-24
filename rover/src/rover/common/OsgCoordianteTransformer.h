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

#pragma once

#include <memory>
#include <osgEarth/GeoTransform>
#include <osgEarth/SpatialReference>
#include "inet/common/INETDefs.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/common/geometry/common/Quaternion.h"

namespace rover {

class OsgCoordianteTransformer {
 public:
  OsgCoordianteTransformer();
  OsgCoordianteTransformer(std::string epgs_code, inet::Coord offset);
  virtual ~OsgCoordianteTransformer();
  inet::Coord convert2D(inet::GeoCoord& c) const;
  inet::Coord convert2D(double lon, double lat) const;
  inet::GeoCoord convertGeo(double x, double y, double z = 0.0) const;
  inet::GeoCoord convertGeo(inet::Coord& c) const;

 private:
  std::string epsg_code;
  inet::Coord offset;
  osgEarth::SpatialReference* c_srs;  // Cartesian
};

}  // namespace rover
