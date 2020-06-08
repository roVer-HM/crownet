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

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/Protocol.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/ModuleOperations.h"

using namespace inet;

namespace rover {

/**
 * Base AID Interface used for message and packet forwarding in the AID Layer.
 * There is no message passing between components of the AID Layer.
 * Implementations of this class must retain module references to each AID
 * componenten and call C++ function directly. All message passing is done by
 * this class or its children.
 */
class IAid : public LayeredProtocolBase {
 public:
  IAid();
  virtual ~IAid();

 protected:
  virtual int numInitStages() const override { return NUM_INIT_STAGES; }
  virtual void registerServiceAndProtocol();

  virtual bool isUpperMessage(cMessage *message) override;
  virtual bool isLowerMessage(cMessage *message) override;

  virtual bool isInitializeStage(int stage) override;
  virtual bool isModuleStartStage(int stage) override;
  virtual bool isModuleStopStage(int stage) override;

 public:
  // custom static const inet::Protocol for AID
  static const Protocol aid;  // adaptive information dissemination

  // init stage for AID
  static const InitStages INITSTAGE_AID_LAYER;
  // start operation stage for AID
  static const ModuleStartOperation::Stage STAGE_AID_PROTOCOLS;
};
}  // namespace rover
