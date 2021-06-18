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

#include <artery/application/Middleware.h>
#include <artery/application/MovingNodeDataProvider.h>
#include "crownet/artery/traci/VaderePersonController.h"

namespace crownet {
class PedestrianMiddlewareEmpty : public artery::MiddlewareBase {
  virtual ~PedestrianMiddlewareEmpty() = default;
  void initialize(int stage) override;

 protected:
  void finish() override;
  void initializeController(omnetpp::cPar&);
  void receiveSignal(omnetpp::cComponent*, omnetpp::simsignal_t,
                     omnetpp::cObject*, omnetpp::cObject*) override;

 private:
  VaderePersonController* mPersonController = nullptr;
  artery::MovingNodeDataProvider mDataProvider;
};
}  // namespace crownet
