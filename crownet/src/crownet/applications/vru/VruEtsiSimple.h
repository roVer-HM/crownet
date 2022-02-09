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

#include "../../mobility/IPositionHistoryProvider.h"
#include "inet/mobility/base/MobilityBase.h"
#include "crownet/applications/common/BaseApp.h"
#include "crownet/common/ItsPdu_m.h"

namespace crownet {

class VruEtsiSimple : public BaseApp {
 public:
  VruEtsiSimple();
  virtual ~VruEtsiSimple();

  //CrownetPacketSourceBase
  virtual Packet *createPacket() override;

 protected:
  IPositionHistoryProvider* mobilityModule = nullptr;

 protected:
  virtual void initialize(int stage) override;
  virtual Coord getCurrentLocation();

  virtual FsmState handleDataArrived(Packet *packet) override;

};
}  // namespace crownet
