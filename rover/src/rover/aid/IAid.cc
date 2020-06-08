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

#include "IAid.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/InitStages.h"

using namespace inet;

namespace rover {

IAid::IAid() {}

IAid::~IAid() {}

void IAid::registerServiceAndProtocol() {
  registerService(IAid::aid, gate("upperIn"), gate("lowerIn"));
  registerProtocol(IAid::aid, gate("lowerOut"), gate("upperOut"));
}

bool IAid::isUpperMessage(cMessage *msg) { return msg->arrivedOn("upperIn"); }

bool IAid::isLowerMessage(cMessage *msg) { return msg->arrivedOn("lowerIn"); }

bool IAid::isInitializeStage(int stage) { return stage == INITSTAGE_AID_LAYER; }

bool IAid::isModuleStartStage(int stage) {
  return stage == STAGE_AID_PROTOCOLS;
}

bool IAid::isModuleStopStage(int stage) { return stage == STAGE_AID_PROTOCOLS; }

// Custom protocol identifiers
const Protocol IAid::aid("aid", "AID");
// use INITSTAGE_ROUTING_PROTOCOLS to be after transport and before
// application.
const InitStages IAid::INITSTAGE_AID_LAYER = INITSTAGE_ROUTING_PROTOCOLS;
// use STAGE_ROUTING_PROTOCOLS to be after transport and before
// application.
const ModuleStartOperation::Stage IAid::STAGE_AID_PROTOCOLS =
    ModuleStartOperation::STAGE_ROUTING_PROTOCOLS;

}  // namespace rover
