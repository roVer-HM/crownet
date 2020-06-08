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

#include "rover/aid/simple/SimpleAid.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/InitStages.h"
#include "inet/common/ModuleAccess.h"

using namespace inet;

namespace rover {

Define_Module(SimpleAid);

SimpleAid::SimpleAid() {}

SimpleAid::~SimpleAid() {}

void SimpleAid::initialize(int stage) {
  if (stage == INITSTAGE_LOCAL) {
    cfg = getModuleFromPar<SimpleAidConfig>(par("aidConfigModule"), this);

  } else if (stage == INITSTAGE_AID_LAYER) {
    // after Transport before Applications.

    // register AID service and protocol for IProtocolRegistrationListener
    // (i.e. Dispatcher)
    registerServiceAndProtocol();
  }
}

// #############################
// life cycle
// #############################

void SimpleAid::handleStartOperation(LifecycleOperation *operation) {
  // todo startup operation
}

void SimpleAid::handleStopOperation(LifecycleOperation *operation) {
  // todo stop operation
}

void SimpleAid::handleCrashOperation(LifecycleOperation *operation) {
  // todo crash operation
}

// GUI
void SimpleAid::refreshDisplay() const {
  OperationalBase::refreshDisplay();

  char buf[80];
  sprintf(buf, "passed up: %d pks\nsent: %d pks", numPassedUp, numSent);
  if (numDroppedWrongPort > 0) {
    sprintf(buf + strlen(buf), "\ndropped (no app): %d pks",
            numDroppedWrongPort);
    getDisplayString().setTagArg("i", 1, "red");
  }
  getDisplayString().setTagArg("t", 0, buf);
}
}  // namespace rover
