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

#include <omnetpp/cexception.h>
#include <memory>
#include <osgEarth/GeoTransform>
#include <osgEarth/SpatialReference>
#include <vanetza/geonet/areas.hpp>
#include "artery/utility/Geometry.h"
#include "inet/common/Units.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "traci/Boundary.h"
#include "traci/GeoPosition.h"
#include "traci/Position.h"

namespace rover {

class OsgCoordinateConverter {
 public:
  OsgCoordinateConverter();
  OsgCoordinateConverter(inet::Coord zoneOriginOffset, inet::Coord simBound,
                         std::string epgs_code);
  OsgCoordinateConverter(traci::TraCIPosition zoneOriginOffset,
                         traci::Boundary simBound, std::string epgs_code);

  virtual ~OsgCoordinateConverter();

  template <typename T>
  artery::Position position_cast_artery(const T& pos) const;

  template <typename T>
  traci::TraCIPosition position_cast_traci(const T& pos) const;

  template <typename T>
  inet::Coord position_cast_inet(const T& pos) const;

  inet::Coord convert2D(inet::GeoCoord& c) const;
  inet::Coord convert2D(double lon, double lat) const;

  template <typename T>
  osgEarth::GeoPoint convert2DOsgEarth(const T& c) const {
    osgEarth::GeoPoint input = addZoneOriginOffset(position_cast_traci(c));
    osgEarth::GeoPoint output = input.transform(c_srs->getGeographicSRS());
    if (output.isValid()) {
      return output;
    } else {
      throw omnetpp::cRuntimeError("invalid transformation");
    }
  }

  template <typename T>
  osgEarth::GeoPoint convertGeoOsgEarth(const T& c) const {
    osgEarth::GeoPoint input = addZoneOriginOffset(position_cast_traci(c));
    osgEarth::GeoPoint output = input.transform(c_srs->getGeographicSRS());
    if (output.isValid()) {
      return output;
    } else {
      throw omnetpp::cRuntimeError("invalid transformation");
    }
  }

  template <typename T>
  inet::GeoCoord convertToGeoInet(const T& c) const {
    osgEarth::GeoPoint output = convertGeoOsgEarth(c);  // (lon,lat,alt)
    return inet::GeoCoord(inet::deg(output.y()), inet::deg(output.x()),
                          inet::m(output.z()));
  }

  template <typename T>
  artery::GeoPosition convertoToGeArtery(const T& c) const {
    osgEarth::GeoPoint output = convertGeoOsgEarth(c);  // (lon,lat,alt)
    artery::GeoPosition geo;
    geo.latitude = output.y() * boost::units::degree::degree;
    geo.longitude = output.x() * boost::units::degree::degree;
    return geo;
  }

  template <typename T>
  traci::TraCIGeoPosition convertToGeoTraCi(const T& c) const {
    osgEarth::GeoPoint output = convertGeoOsgEarth(c);  // (lon,lat,alt)
    traci::TraCIGeoPosition geo;
    geo.longitude = output.x();
    geo.latitude = output.y();
    return geo;
  }

  template <typename T>
  osgEarth::GeoPoint geoToOsgEarth(const T& c) const;

  template <typename T>
  osgEarth::GeoPoint convertToCartGeoPoint(
      const T& c, const bool applyZoneOffset = true) const {
    osgEarth::GeoPoint geoInput = geoToOsgEarth(c);
    osgEarth::GeoPoint output = geoInput.transform(c_srs);
    if (output.isValid()) {
      if (applyZoneOffset) removeZoneOriginOffset(output);
      return output;
    } else {
      throw omnetpp::cRuntimeError("invalid transformation");
    }
  }

  template <typename T>
  inet::Coord convertToCartInetPosition(const T& c) const {
    osgEarth::GeoPoint output = convertToCartGeoPoint(c);
    return position_cast_inet(output);
  }

  template <typename T>
  traci::TraCIPosition convertToCartTraCIPosition(const T& c) const {
    osgEarth::GeoPoint output = convertToCartGeoPoint(c);
    return position_cast_traci(output);
  }

  template <typename T>
  artery::Position convertToCartArteryPosition(const T& c) const {
    osgEarth::GeoPoint output = convertToCartGeoPoint(c);
    return position_cast_artery(output);
  }

  double getBoundaryWidth() const;
  double getBoundaryHeight() const;

 private:
  osgEarth::GeoPoint addZoneOriginOffset(const traci::TraCIPosition& pos) const;
  void removeZoneOriginOffset(osgEarth::GeoPoint& pos) const;

  // TraCI coordinate system (TCS):
  //  * right-hand-rule z-positive *OUT* of plane.
  //  * origin lower left corner of canvas
  //  * x positive to right
  //  * y positive up
  // OMNeT++ coordinate system (OCS)
  //  * right-hand-rule z-positive *INTO* of plane.
  //  * origin upper left corner of canvas
  //  * x positive to right
  //  * y positive down
  traci::TraCIPosition zoneOriginOffset;  // TCS base
  traci::Boundary simBound;               // TCS base
  std::string epsg_code;
  osgEarth::SpatialReference*
      c_srs;  // Cartesian TCS base (with zoneOriginOffset applied)
};

template <>
inline osgEarth::GeoPoint OsgCoordinateConverter::convertGeoOsgEarth(
    const traci::TraCIPosition& c) const {
  // no position_cast_needed
  osgEarth::GeoPoint input = addZoneOriginOffset(c);
  osgEarth::GeoPoint output = input.transform(c_srs->getGeographicSRS());
  if (output.isValid()) {
    return output;
  } else {
    throw omnetpp::cRuntimeError("invalid transformation");
  }
}

template <>
inline artery::Position OsgCoordinateConverter::position_cast_artery(
    const traci::TraCIPosition& pos) const {
  // TCS->OCS
  return traci::position_cast(simBound, pos);
}

template <>
inline artery::Position OsgCoordinateConverter::position_cast_artery(
    const inet::Coord& pos) const {
  // (OCS->OCS)
  return artery::Position(pos.x, pos.y);
}

template <>
inline artery::Position OsgCoordinateConverter::position_cast_artery(
    const osgEarth::GeoPoint& pos) const {
  // osgCoord always represents TCS: TCS->OCS
  const double x = pos.x() - simBound.lowerLeftPosition().x;
  const double y = simBound.upperRightPosition().y - pos.y();
  return artery::Position(x, y);
}

template <>
inline traci::TraCIPosition OsgCoordinateConverter::position_cast_traci(
    const artery::Position& pos) const {
  // OCS->TCS
  return traci::position_cast(simBound, pos);
}

template <>
inline traci::TraCIPosition OsgCoordinateConverter::position_cast_traci(
    const inet::Coord& pos) const {
  // OCS->TCS
  const double x = pos.x + simBound.lowerLeftPosition().x;
  const double y = simBound.upperRightPosition().y - pos.y;
  const double z = pos.z;
  traci::TraCIPosition tmp;
  tmp.x = x;
  tmp.y = y;
  tmp.z = z;
  return tmp;
}

template <>
inline traci::TraCIPosition OsgCoordinateConverter::position_cast_traci(
    const osgEarth::GeoPoint& pos) const {
  // osgCoord always represents TCS: (TCS -> TCS) no translation needed
  traci::TraCIPosition tmp;
  tmp.x = pos.x();
  tmp.y = pos.y();
  tmp.z = pos.z();
  return tmp;
}

template <>
inline inet::Coord OsgCoordinateConverter::position_cast_inet(
    const traci::TraCIPosition& pos) const {
  // TCS->OCS
  const double x = pos.x - simBound.lowerLeftPosition().x;
  const double y = simBound.upperRightPosition().y - pos.y;
  const double z = pos.z;
  return inet::Coord(x, y, z);
}

template <>
inline inet::Coord OsgCoordinateConverter::position_cast_inet(
    const artery::Position& pos) const {
  // (OCS->OCS)
  return inet::Coord(pos.x.value(), pos.y.value(), 0.0);
}

template <>
inline inet::Coord OsgCoordinateConverter::position_cast_inet(
    const osgEarth::GeoPoint& pos) const {
  // osgCoord always represents TCS: (TCS -> OCS)
  const double x = pos.x() - simBound.lowerLeftPosition().x;
  const double y = simBound.upperRightPosition().y - pos.y();
  const double z = pos.z();
  return inet::Coord(x, y, z);
}

template <>
inline osgEarth::GeoPoint OsgCoordinateConverter::geoToOsgEarth(
    const inet::GeoCoord& c) const {
  return osgEarth::GeoPoint{c_srs->getGeographicSRS(), c.longitude.get(),
                            c.latitude.get(), c.altitude.get()};
}

template <>
inline osgEarth::GeoPoint OsgCoordinateConverter::geoToOsgEarth(
    const traci::TraCIGeoPosition& c) const {
  return osgEarth::GeoPoint{c_srs->getGeographicSRS(), c.longitude, c.latitude,
                            0.0};
}

template <>
inline osgEarth::GeoPoint OsgCoordinateConverter::geoToOsgEarth(
    const artery::GeoPosition& c) const {
  return osgEarth::GeoPoint{c_srs->getGeographicSRS(), c.longitude.value(),
                            c.latitude.value(), 0.0};
}

template <>
inline osgEarth::GeoPoint OsgCoordinateConverter::geoToOsgEarth(
    const vanetza::geonet::GeodeticPosition& c) const {
  return osgEarth::GeoPoint{c_srs->getGeographicSRS(), c.longitude.value(),
                            c.latitude.value(), 0.0};
}

}  // namespace rover
