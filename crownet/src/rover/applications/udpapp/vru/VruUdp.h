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

#include "inet/mobility/base/MobilityBase.h"
#include "rover/applications/common/UdpBaseApp.h"

namespace rover {

class VruUdp : public UdpBaseApp {
 public:
  VruUdp();
  virtual ~VruUdp();

 protected:
  MobilityBase *mobilityModule = nullptr;

 protected:
  virtual void initialize(int stage) override;
  virtual FsmState fsmAppMain(cMessage *msg) override;
  virtual Coord getCurrentLocation();
};
}  // namespace rover
