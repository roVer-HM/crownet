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
#include "rover/aid/IAid.h"
#include "rover/aid/simple/SimpleAidConfig.h"

using namespace inet;

namespace rover {
class SimpleAid : public IAid {
 public:
  SimpleAid();
  virtual ~SimpleAid();

 protected:
  virtual void initialize(int stage) override;
  virtual void refreshDisplay() const override;

  // ILifeCycle:
  virtual void handleStartOperation(LifecycleOperation *operation) override;
  virtual void handleStopOperation(LifecycleOperation *operation) override;
  virtual void handleCrashOperation(LifecycleOperation *operation) override;

 protected:
  SimpleAidConfig *cfg = nullptr;  // managed by parent do not delete

  // statistics
  int numSent = 0;
  int numPassedUp = 0;
  int numDroppedWrongPort = 0;
  int numDroppedBadChecksum = 0;
};
}  // namespace rover
