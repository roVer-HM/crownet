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


inet::Coord Projection::compute(const inet::Coord& point) const{
    inet::Coord rotatedPoint = rotation.rotateVector(point);
    return inet::Coord(rotatedPoint.x * scale.x + translation.x,
                       rotatedPoint.y * scale.y + translation.y,
                       rotatedPoint.z * scale.z + translation.z );

}

inet::Coord Projection::computeInverse(const inet::Coord& point) const{
    inet::Coord p((point.x - translation.x) / scale.x, (point.y - translation.y) / scale.y, (point.z - translation.z) / scale.z);
    return rotation.rotateVectorInverse(p);
}

traci::TraCIPosition Projection::compute(const traci::TraCIPosition& p) const{
    inet::Coord rotatedPoint = rotation.rotateVector(inet::Coord(p.x, p.y, p.z));
    return traci::TraCIPosition(rotatedPoint.x * scale.x + translation.x,
                               rotatedPoint.y * scale.y + translation.y,
                               rotatedPoint.z * scale.z + translation.z );

}


traci::TraCIPosition Projection::computeInverse(const traci::TraCIPosition& point) const{
    inet::Coord p((point.x - translation.x) / scale.x, (point.y - translation.y) / scale.y, (point.z - translation.z) / scale.z);
    p = rotation.rotateVectorInverse(p);
    return traci::TraCIPosition(p.x, p.y, p.z);
}




OsgCoordinateConverter::~OsgCoordinateConverter() {}

OsgCoordinateConverter::OsgCoordinateConverter(
    traci::TraCIPosition zoneOriginOffset, traci::Boundary simBound,
    std::string srs_code)
    : zoneOriginOffset(zoneOriginOffset),
      simBound(simBound),
      areaOfIntrest(simBound),
      c_srs(osgEarth::SpatialReference::get(srs_code)){

    zoneOffsetProjection.setTranslation(inet::Coord(zoneOriginOffset.x, zoneOriginOffset.y, zoneOriginOffset.z));
}

OsgCoordinateConverter::OsgCoordinateConverter(inet::Coord zoneOriginOffset,
                                               inet::Coord _simBound,
                                               std::string srs_code){
  this->zoneOriginOffset.x = zoneOriginOffset.x;
  this->zoneOriginOffset.y = zoneOriginOffset.y;
  this->zoneOriginOffset.z = zoneOriginOffset.z;
  zoneOffsetProjection.setTranslation(inet::Coord(zoneOriginOffset.x, zoneOriginOffset.y, zoneOriginOffset.z));

  std::vector<traci::TraCIPosition> vec;
  traci::TraCIPosition upperRight{_simBound.x, _simBound.y, 0.0};
  vec.push_back(traci::TraCIPosition{0.0, 0.0, 0.0});
  vec.push_back(upperRight);
  simBound = traci::Boundary(vec);
  areaOfIntrest = traci::Boundary(vec);

  this->c_srs = osgEarth::SpatialReference::get(srs_code);
}

double OsgCoordinateConverter::getBoundaryWidth() const {
  return std::abs(simBound.upperRightPosition().x -
                  simBound.lowerLeftPosition().x);
}
double OsgCoordinateConverter::getBoundaryHeight() const {
  return std::abs(simBound.upperRightPosition().y -
                  simBound.lowerLeftPosition().y);
}

inet::Coord OsgCoordinateConverter::getBoundary() const{
    return inet::Coord{getBoundaryWidth(), getBoundaryHeight(), 0.0};
}

inet::Coord OsgCoordinateConverter::getOffset() const {
    return inet::Coord{this->zoneOriginOffset.x, this->zoneOriginOffset.y};
}


RegularGridInfo OsgCoordinateConverter::getGridDescription() const{
    return getGridDescription(this->cellSize);
}


RegularGridInfo OsgCoordinateConverter::getGridDescription(const inet::Coord& cellSize) const {
    // set gridSize to max value of bound.
    auto gridSize = inet::Coord(simBound.upperRightPosition().x, simBound.upperRightPosition().y);
    RegularGridInfo info;
    info.setGridSize(gridSize);
    info.setCellSize(cellSize);
    // use ceil to ensure all possible locations are present.
    info.setCellCount(inet::Coord{std::ceil(gridSize.x / cellSize.x), std::ceil(gridSize.y / cellSize.y), 0.0});
    info.setAreaOfIntrest(areaOfIntrest);
    info.setSimBound(simBound);
    return info;
}

RegularGridInfo OsgCoordinateConverter::getGridDescription(const double cellSize) const{
    return getGridDescription({cellSize, cellSize, 0.0});
}

osgEarth::GeoPoint OsgCoordinateConverter::addZoneOriginOffset(
    const traci::TraCIPosition& pos) const {
  auto realPos = zoneOffsetProjection.computeInverse(pos); //revert zone offset
  return osgEarth::GeoPoint{c_srs, realPos.x, realPos.y, realPos.z};
}

void OsgCoordinateConverter::removeZoneOriginOffset(
    osgEarth::GeoPoint& pos) const {
  pos.x() -= zoneOriginOffset.x;
  pos.y() -= zoneOriginOffset.y;
  pos.z() -= zoneOriginOffset.z;
}

inet::Coord OsgCoordinateConverter::convert2D(const inet::GeoCoord& c, const bool project) const {
  return convert2D(c.latitude.get(), c.longitude.get(), project);
}


inet::Coord OsgCoordinateConverter::convert2D(double lat, double lon, const bool project) const {
  osgEarth::GeoPoint input{c_srs->getGeographicSRS(), lon,
                           lat};  // order!! lon first
  osgEarth::GeoPoint output = input.transform(c_srs);
  inet::Coord ret;
  if (output.isValid()) {
    ret = zoneOffsetProjection.compute(inet::Coord(output.x(), output.y(), output.z()));
  } else {
    throw cRuntimeError("invalid transformation");
  }
  if (project){
      ret = moveCoordinateSystemTraCi_Opp(ret);
  }
  return ret;
}

inet::Coord OsgCoordinateConverter::moveCoordinateSystemTraCi_Opp(const inet::Coord& c) const{
    inet::Coord ret;
    ret.x = c.x - simBound.lowerLeftPosition().x;
    ret.y = simBound.upperRightPosition().y - c.y;
    ret.z = c.z;
    return ret;
}

inet::Coord OsgCoordinateConverter::moveCoordinateSystemOpp_TraCi(const inet::Coord& c) const{
    inet::Coord ret;
    ret.x = c.x + simBound.lowerLeftPosition().x;
    ret.y = simBound.upperRightPosition().y - c.y;
    ret.z = c.z;
    return ret;
}


inet::GeoCoord OsgCoordinateConverter::getScenePosition() const{
    // Scene position is allays OCS (opp Cartesian with origin at top/left)
    inet::Coord origin(0.0, 0.0, 0.0);
    return convertToGeoInet(origin);
}

void OsgCoordinateConverter::setAreaOfInterest(AreaOfInterest* aoi) {
    if (aoi != nullptr){
        std::vector<traci::TraCIPosition> vec;
        traci::TraCIPosition upperRight{aoi->getX()+aoi->getWidth(), aoi->getY()+aoi->getHeight(), 0.0};
        vec.push_back(traci::TraCIPosition{aoi->getX(), aoi->getY(), 0.0});
        vec.push_back(upperRight);
        auto _aoi = traci::Boundary(vec);
        if (_aoi.lowerLeftPosition().x >= simBound.lowerLeftPosition().x &&
                _aoi.lowerLeftPosition().y >= simBound.lowerLeftPosition().y &&
                _aoi.upperRightPosition().x <= simBound.upperRightPosition().x &&
                _aoi.upperRightPosition().y <= simBound.upperRightPosition().y
        ){
            areaOfIntrest = _aoi;
        } else {
            throw cRuntimeError("AreaOfIntrest (AOI) outside of simulation bound");
        }
    }
}

const bool OsgCoordinateConverter::aoiContains(const traci::TraCIPosition& p) const {
    return areaOfIntrest.lowerLeftPosition().x <= p.x &&
            areaOfIntrest.upperRightPosition().x <= p.x &&
            areaOfIntrest.lowerLeftPosition().y <= p.y &&
            areaOfIntrest.upperRightPosition().y <= p.y;
}
const bool OsgCoordinateConverter::aoiContains(const inet::Coord& c) const {
    return aoiContains(position_cast_traci(c));
}

const bool OsgCoordinateConverter::aoiIntersectsCell(const GridCellID& cellId) const {
    return aoiIntersectsCell(cellId.x(), cellId.y());
}

const bool OsgCoordinateConverter::aoiIntersectsCell(const int x, const int y) const {
    return aoiContains(traci::TraCIPosition(x*cellSize.x, y*cellSize.y)) ||
            aoiContains(traci::TraCIPosition((x+1)*cellSize.x, y*cellSize.y)) ||
            aoiContains(traci::TraCIPosition((x+1)*cellSize.x, (y+1)*cellSize.y)) ||
            aoiContains(traci::TraCIPosition(x*cellSize.x, (y+1)*cellSize.y));
}


// always apply TCS->OCS
const omnetpp::cFigure::Point OsgCoordinateConverter::toCanvas(double x, double y, const bool isGeo) const{
    if (isGeo){
        throw cRuntimeError("not implemented");
    } else {
        auto p = moveCoordinateSystemTraCi_Opp(inet::Coord(x, y));
        return cFigure::Point(p.x, p.y);
    }
}
const omnetpp::cFigure::Point OsgCoordinateConverter::toCanvas(traci::TraCIPosition pos) const{
    return toCanvas(pos.x, pos.y, false);
}

cRectangleFigure* OsgCoordinateConverter::toCanvas(const traci::Boundary& _bound, const char *name) const {
    auto l1 = toCanvas(_bound.lowerLeftPosition());
    auto l2 = toCanvas(_bound.upperRightPosition());
    auto *rect = new cRectangleFigure(name);
    rect->setBounds(cFigure::Rectangle(l1.x, l2.y, abs(l2.x-l1.x), abs(l2.y-l1.y)));
    return rect;
}

cRectangleFigure* OsgCoordinateConverter::toCanvas(const GridCellID& cellID) const {
    auto p = toCanvas(cellID.x()*cellSize.x, (cellID.y()+1)*cellSize.y, false);
    auto *rect = new cRectangleFigure();
    rect->setBounds(cFigure::Rectangle(p.x, p.y, cellSize.x, cellSize.y));
    return rect;
}


}  // namespace crownet
