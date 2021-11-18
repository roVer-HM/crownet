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
#include <omnetpp/ccanvas.h>
#include <memory>
#include <osgEarth/GeoTransform>
#include <osgEarth/SpatialReference>
#include <vanetza/geonet/areas.hpp>
#include <artery/traci/Cast.h>
#include "artery/utility/Geometry.h"
#include "inet/common/Units.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "traci/Boundary.h"
#include "traci/GeoPosition.h"
#include "traci/Position.h"
#include "inet/common/geometry/common/RotationMatrix.h"
#include "crownet/common/RegularGridInfo.h"


namespace crownet {


class Projection {
  protected:
    inet::RotationMatrix rotation;
    inet::Coord scale;
    inet::Coord translation;
  public:
    Projection(): scale(inet::Coord(1.0, 1.0, 1.0)){}
    Projection(inet::RotationMatrix rotation, inet::Coord scale, inet::Coord translation):
        rotation(rotation), scale(scale), translation(translation){}

    const inet::RotationMatrix& getRotation() const { return rotation; }
    void setRotation(const inet::RotationMatrix& rotation) { this->rotation = rotation; }

    const inet::Coord& getScale() const { return scale; }
    void setScale(const inet::Coord& scale) { this->scale = scale; }

    const inet::Coord& getTranslation() const { return translation; }
    void setTranslation(const inet::Coord& translation) { this->translation = translation; }



    inet::Coord compute(const inet::Coord& point) const;
    inet::Coord computeInverse(const inet::Coord& point) const;

    traci::TraCIPosition compute(const traci::TraCIPosition& point) const;
    traci::TraCIPosition computeInverse(const traci::TraCIPosition& point) const;
};


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

  inet::Coord convert2D(const inet::GeoCoord& c, const bool project = false) const;
  inet::Coord convert2D(double lon, double lat, const bool project = false) const;

  inet::Coord moveCoordinateSystemTraCi_Opp(const inet::Coord& c) const;
  inet::Coord moveCoordinateSystemOpp_TraCi(const inet::Coord& c) const;

  inet::GeoCoord getScenePosition() const;

  double getBoundaryWidth() const;
  double getBoundaryHeight() const;
  inet::Coord getBoundary() const;
  inet::Coord getOffset() const;
  RegularGridInfo getGridDescription(const inet::Coord& cellSize) const;
  RegularGridInfo getGridDescription(const double cellSize) const;



  // always apply TCS->OCS
  const omnetpp::cFigure::Point toCanvas(double x, double y, const bool isGeo=false);


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


 private:
  osgEarth::GeoPoint addZoneOriginOffset(const traci::TraCIPosition& pos) const;
  void removeZoneOriginOffset(osgEarth::GeoPoint& pos) const;

  // TraCI Cartesian system (TCS):
  //  * right-hand-rule z-positive *OUT* of plane.
  //  * origin lower left corner of canvas
  //  * x positive to right
  //  * y positive up
  // OMNeT++ Cartesian system (OCS)
  //  * right-hand-rule z-positive *INTO* of plane.
  //  * origin upper left corner of canvas
  //  * x positive to right
  //  * y positive down
  traci::TraCIPosition zoneOriginOffset;  // TCS base
  traci::Boundary simBound;               // TCS base
  osgEarth::SpatialReference* c_srs;
  Projection zoneOffsetProjection;
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
  return artery::position_cast(simBound, pos);
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
  return artery::position_cast(simBound, pos);
}

template <>
inline traci::TraCIPosition OsgCoordinateConverter::position_cast_traci(
    const inet::Coord& pos) const {
  // OCS->TCS
  auto posProjected = moveCoordinateSystemOpp_TraCi(pos);
  traci::TraCIPosition tmp;
  tmp.x = posProjected.x;
  tmp.y = posProjected.y;
  tmp.z = posProjected.z;
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

    return moveCoordinateSystemTraCi_Opp(inet::Coord(pos.x, pos.y, pos.z));
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
    return moveCoordinateSystemTraCi_Opp(inet::Coord(pos.x(), pos.y(), pos.z()));
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

}  // namespace crownet
