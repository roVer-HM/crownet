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

#include "rover_util.h"

namespace rover {

PathPoint::PathPoint(inet::Coord referencePoint,
                     omnetpp::simtime_t referenceTime) {
  this->referencePoint = referencePoint;
  this->referenceTime = referenceTime;
}

PathPoint PathPoint::operator+(const PathPoint& other) {
  return PathPoint(this->referencePoint + other.referencePoint,
                   this->referenceTime + other.referenceTime);
}

PathPoint PathPoint::operator-(const PathPoint& other) {
  return PathPoint(this->referencePoint - other.referencePoint,
                   this->referenceTime - other.referenceTime);
}

PathPoint& PathPoint::operator=(const PathPoint& other) {
  PathPoint_Base::operator=(other);
  return *this;
}

}  // namespace rover
