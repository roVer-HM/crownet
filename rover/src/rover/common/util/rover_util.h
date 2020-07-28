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

#include "rover/common/util/rover_util_m.h"

namespace rover {
class ROVER_API PathPoint : public PathPoint_Base {
 public:
  PathPoint() : PathPoint_Base(){};
  PathPoint(inet::Coord referencePoint, omnetpp::simtime_t referenceTime);

  PathPoint operator+(const PathPoint& other);
  PathPoint operator-(const PathPoint& other);
  PathPoint& operator=(const PathPoint& other);
};

}  // namespace rover
