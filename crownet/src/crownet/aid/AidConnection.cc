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

#include "AidConnection.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/Simsignals_m.h"
#include "crownet/aid/AidCommand_m.h"
#include "crownet/aid/AidHeader_m.h"
#include "crownet/applications/udpapp/detour/DetourAppPacket_m.h"
#include "crownet/common/SocketHandler.h"

using namespace inet;

namespace crownet {

Define_Module(AidConnection);

AidConnection::AidConnection() {
  aidConnectionId = getEnvir()->getUniqueNumber();
}

AidConnection::~AidConnection() {
  // TODO Auto-generated destructor stub
}

void AidConnection::initConnection(Aid* aidMain, int appSocketId) {
  Enter_Method_Silent();

  this->aidMain = aidMain;
  this->appSocketId = appSocketId;

  fsm.setName(getName());
  fsm.setState(AID_S_INIT);

  // create timers (selfMessages)
}

int AidConnection::getAppSocketId() const { return appSocketId; }

int AidConnection::getTransSocketId() const {
  return socketHandler == nullptr ? -1 : socketHandler->getSocketId();
}

int AidConnection::getAidConnectionId() const { return aidConnectionId; }

const L3Address& AidConnection::getLocalAddr() const { return localAddr; }

const L3Address& AidConnection::getRemoteAddr() const { return remoteAddr; }

/** Message processing */
void AidConnection::handleMessage(cMessage* msg) {
  if (msg->isSelfMessage()) {
    if (!processTimer(msg)) aidMain->removeConnection(this);
  } else
    throw cRuntimeError("model error: AidConnection allows only self messages");
}

bool AidConnection::processAppCommand(cMessage* msg) {
  Enter_Method_Silent();

  printConnBrief();

  AidCommand* aidCmd =
      check_and_cast_nullable<AidCommand*>(msg->removeControlInfo());
  AidEventCode event = preanalyseAppCommandEvent(msg->getKind());
  EV_INFO << "App command: " << eventName(event) << "\n";

  switch (event) {
    case AID_E_BIND:
      process_AID_BIND(event, aidCmd, msg);
      break;
    case AID_E_APP_REQ:
      process_AID_APP_REQ(event, aidCmd, msg);
      break;
    case AID_E_APP_CAP:
      process_AID_APP_CAP(event, aidCmd, msg);
      break;
    case AID_E_CONNECT:
      process_AID_CONNECT(event, aidCmd, msg);
      break;
    case AID_E_DATA:
      process_AID_DATA(event, aidCmd, msg);
      break;
    case AID_E_CLOSE:
      process_AID_CLOSE(event, aidCmd, msg);
      break;
    case AID_E_DESTROY:
      process_AID_DESTROY(event, aidCmd, msg);
      break;
    default:
      throw cRuntimeError(aidMain, "wrong event code");
  }
  return performStateTransition(event);
}

bool AidConnection::processLower(Packet* packet, int receivingSocketId) {
  //  auto appPkt = packet->popAtFront<ApplicationPacket>();
  auto aidHeader = packet->popAtFront<AidHeader>();
  if (aidHeader) {
    // emit rcv...
    packet->setKind(AID_I_DATA);
    packet->removeTagIfPresent<SocketInd>();
    packet->addTag<SocketInd>()->setSocketId(appSocketId);
    sendToApp(packet);

  } else {
    PacketDropDetails details;
    details.setReason(NO_PROTOCOL_FOUND);
    aidMain->emit(packetDroppedSignal, packet, &details);
    EV_ERROR << "wrong protocol received. Expected aid" << std::endl;
    delete packet;
  }
  return true;
}

bool AidConnection::processTimer(cMessage* msg) {
  throw cRuntimeError("Timer processing not implemented!");
}
/** Message processing End*/

/** State Machine Handling */
AidEventCode AidConnection::preanalyseAppCommandEvent(int commandCode) {
  switch (commandCode) {
    case AID_C_BIND:
      return AID_E_BIND;
    case AID_C_APP_REQ:
      return AID_E_APP_REQ;
    case AID_C_APP_CAP:
      return AID_E_APP_CAP;
    case AID_C_CONNECT:
      return AID_E_CONNECT;
    case AID_C_DATA:
      return AID_E_DATA;
    case AID_C_CLOSE:
      return AID_E_CLOSE;
    case AID_C_DESTROY:
      return AID_E_DESTROY;
    default:
      throw cRuntimeError(aidMain, "Unknown message kind in app command");
  }
}

bool AidConnection::performStateTransition(const AidEventCode& event) {
  ASSERT(fsm.getState() !=
         AID_S_CLOSED);  // closed connections should be deleted immediately

  if (event == AID_E_IGNORE) {  // e.g. discarded segment
    EV_DETAIL << "Staying in state: " << stateName(fsm.getState())
              << " (no FSM event)\n";
    return true;
  }

  int oldState = fsm.getState();

  switch (fsm.getState()) {
    case AID_S_INIT:
      switch (event) {
        case AID_E_BIND:
          FSM_Goto(fsm, AID_S_BIND);
          break;
        default:
          throw cRuntimeError(
              "Illegal Event in state AID_S_INIT. Expected AID_E_BIND. Did you "
              "call bind()?");
      }
      break;

    case AID_S_BIND:
      switch (event) {
        case AID_E_APP_REQ:
        case AID_E_APP_CAP:
          FSM_Goto(fsm, AID_S_BIND);  // stay in AID_S_BIND
          break;
        case AID_E_CONNECT:
          FSM_Goto(fsm, AID_S_ESTABLISHED);
          break;
        default:
          throw cRuntimeError(
              "Illegal Event in state AID_S_BIND. Expected AID_E_APP_REQ, "
              "AID_E_APP_CAP or AID_E_CONNECT.");
      }
      break;

    case AID_S_ESTABLISHED:
      switch (event) {
        case AID_E_DATA:
        case AID_E_APP_REQ:
        case AID_E_APP_CAP:
          FSM_Goto(fsm, AID_S_ESTABLISHED);  // stay in AID_S_ESTABLISHED
          break;
        case AID_E_CLOSE:
          FSM_Goto(fsm, AID_S_CLOSED);
          break;
        case AID_E_DESTROY:
          FSM_Goto(fsm, AID_S_DESTROYED);
          break;
        case AID_E_CONNECT:
        case AID_E_BIND:
        default:
          throw cRuntimeError("Illegal Event in AID_S_ESTABLISHED");
      }
      break;

    case AID_S_CLOSED:
      break;
  }

  if (oldState != fsm.getState()) {
    EV_INFO << "Transition: " << stateName(oldState) << " --> "
            << stateName(fsm.getState()) << "  (event was: " << eventName(event)
            << ")\n";
    EV_DEBUG_C("testing") << aidMain->getName() << ": " << stateName(oldState)
                          << " --> " << stateName(fsm.getState()) << "  (on "
                          << eventName(event) << ")\n";

    // cancel timers, etc.
    stateEntered(fsm.getState(), oldState, event);
  } else {
    EV_DETAIL << "Staying in state: " << stateName(fsm.getState())
              << " (event was: " << eventName(event) << ")\n";
  }

  return fsm.getState() != AID_S_CLOSED;
}

void AidConnection::stateEntered(int state, int oldState, AidEventCode event) {
  // cancel timers
  switch (state) {
    case AID_S_INIT:
      break;
    case AID_S_CLOSED:
      break;
    case AID_S_BIND:
      break;
    case AID_S_ESTABLISHED:
      break;
    case AID_S_DESTROYED:
      break;
  }
}

/** State Machine Handling End*/

/** Process App Command (Sate Machine) */
void AidConnection::process_AID_BIND(AidEventCode& event,
                                     AidCommand* aidCommand, cMessage* msg) {
  // AID_S_INIT ==AID_E_BIND==> AID_S_BIND
  AidBindCommand* cmd = check_and_cast<AidBindCommand*>(aidCommand);

  switch (fsm.getState()) {
    case AID_S_INIT:
      localAddr = cmd->getLocalAddr();
      localPort = cmd->getLocalPort();
      break;
    default:
      throw cRuntimeError(
          aidMain, "Error Processing AID_BIND: AidConnection already exists.");
  }
  delete cmd;
  delete msg;
}

void AidConnection::process_AID_APP_REQ(AidEventCode& event,
                                        AidCommand* aidCommand, cMessage* msg) {
  // AID_S_BIND ==AID_E_APP_REQ==> AID_S_BIND
  // AID_S_ESTABLISHED ==AID_E_APP_REQ==> AID_S_ESTABLISHED
  AidAppReqCommand* cmd = check_and_cast<AidAppReqCommand*>(aidCommand);

  switch (fsm.getState()) {
    case AID_S_BIND:
      remoteAddr = cmd->getRemoteAddr();
      remotePort = cmd->getRemotePort();
      appRequirments.maxRate = cmd->getMaxRate();
      appRequirments.minRate = cmd->getMinRate();
      appRequirments.recipientClass = cmd->getRecipientClass();
      break;
    case AID_S_ESTABLISHED:
      // todo: dynamically change requirements
      break;
    default:
      throw cRuntimeError(aidMain,
                          "Error Processing AID_APP_REQ: AidConnection not in "
                          "AID_S_BIND state.");
  }
  delete cmd;
  delete msg;
}

void AidConnection::process_AID_APP_CAP(AidEventCode& event,
                                        AidCommand* aidCommand, cMessage* msg) {
  // AID_S_BIND ==AID_E_APP_REQ==> AID_S_BIND
  // AID_S_ESTABLISHED ==AID_E_APP_REQ==> AID_S_ESTABLISHED
  AidAppCapCommand* cmd = check_and_cast<AidAppCapCommand*>(aidCommand);

  switch (fsm.getState()) {
    case AID_S_BIND:
      // todo: save application capabilities to use in AID algorithms
      break;
    case AID_S_ESTABLISHED:
      // todo: dynamically change capabilities
      break;
    default:
      throw cRuntimeError(aidMain,
                          "Error Processing AID_APP_CAP: AidConnection not in "
                          "AID_S_BIND state.");
  }
  delete cmd;
  delete msg;
}

void AidConnection::process_AID_CONNECT(AidEventCode& event,
                                        AidCommand* aidCommand, cMessage* msg) {
  // AID_S_BIND ==AID_E_CONNECT==> AID_S_ESTABLISHED
  AidConnectCommand* cmd = check_and_cast<AidConnectCommand*>(aidCommand);

  switch (fsm.getState()) {
    case AID_S_BIND:
      // create socket based on recipientClass for know.
      switch (appRequirments.recipientClass) {
        case AidRecipientClass::ALL_LOCAL:
        case AidRecipientClass::SOME_LOCAL:
        case AidRecipientClass::ONE_LOCAL:
          socketHandler = aidMain->createSocketHandler(this, SocketType::UDP);
          break;
        default:
          throw cRuntimeError("No right setting found.");
      }
      break;
    default:
      throw cRuntimeError(aidMain,
                          "Error Processing AID_APP_CAP: AidConnection not in "
                          "AID_S_BIND state.");
  }
  delete cmd;
  delete msg;
}

void AidConnection::process_AID_DATA(AidEventCode& event,
                                     AidCommand* aidCommand, cMessage* msg) {
  // AID_S_ESTABLISHED ==AID_E_DATA==> AID_S_ESTABLISHED

  switch (fsm.getState()) {
    case AID_S_ESTABLISHED: {
      // add PacketProtocolTag to App Packet.
      auto aidHeader = makeShared<AidHeader>();
      aidHeader->setChunkLength(B(8));
      Packet* pkt = check_and_cast<Packet*>(msg);
      // todo: AidProtocolInd do i need this?
      pkt->insertAtFront(aidHeader);
      socketHandler->sendTo(pkt, remoteAddr, remotePort);
      // todo: action or error for each steady state
      break;
    }
    default:
      throw cRuntimeError(aidMain, "Err");
  }
}

void AidConnection::process_AID_CLOSE(AidEventCode& event,
                                      AidCommand* aidCommand, cMessage* msg) {
  AidCloseCommand* cmd = check_and_cast<AidCloseCommand*>(aidCommand);

  switch (fsm.getState()) {
    // todo: action or error for each steady state
    default:
      throw cRuntimeError(aidMain, "Err");
  }

  delete cmd;
  delete msg;
}

void AidConnection::process_AID_DESTROY(AidEventCode& event,
                                        AidCommand* aidCommand, cMessage* msg) {
  delete aidCommand;
  delete msg;
}
/** Process App Command (Sate Machine) End*/

/** Utils */
void AidConnection::sendToApp(cMessage* msg) {
  aidMain->sendFromConn(msg, "upperOut");
}

void AidConnection::printConnBrief() const {
  EV_DETAIL << "Connection " << localAddr << ":" << localPort << " to "
            << remoteAddr << ":" << remotePort << " on socketId=" << appSocketId
            << " in " << stateName(fsm.getState()) << "\n";
}

const char* AidConnection::stateName(int state) {
#define CASE(x)        \
  case x:              \
    s = (char*)#x + 6; \
    break
  const char* s = "unknown";
  switch (state) {
    CASE(AID_S_INIT);
    CASE(AID_S_CLOSED);
    CASE(AID_S_BIND);
    CASE(AID_S_ESTABLISHED);
    CASE(AID_S_DESTROYED);
  }
  return s;
#undef CASE
}

const char* AidConnection::eventName(int event) {
#define CASE(x)        \
  case x:              \
    s = (char*)#x + 6; \
    break
  const char* s = "unknown";
  switch (event) {
    CASE(AID_E_IGNORE);
    CASE(AID_E_BIND);
    CASE(AID_E_APP_REQ);
    CASE(AID_E_APP_CAP);
    CASE(AID_E_CONNECT);
    CASE(AID_E_DATA);
    CASE(AID_E_CLOSE);
    CASE(AID_E_DESTROY);
  }
  return s;
#undef CASE
}

const char* AidConnection::indicationName(int code) {
#define CASE(x)        \
  case x:              \
    s = (char*)#x + 6; \
    break
  const char* s = "unknown";
  switch (code) {
    CASE(AID_I_DATA);
    CASE(AID_I_ERROR);
    CASE(AID_I_STATUS);
    CASE(AID_I_CLOSE);
  }
  return s;
#undef CASE
}
/** Utility End*/

}  // namespace crownet
