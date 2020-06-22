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

#include "BaseApp.h"

using namespace inet;

namespace rover {
BaseApp::~BaseApp() {
  if (selfMsgAppTimer) cancelAndDelete(selfMsgAppTimer);
}

void BaseApp::initialize(int stage) {
  ApplicationBase::initialize(stage);
  if (stage == INITSTAGE_LOCAL) {
    numSent = 0;
    numReceived = 0;
    WATCH(numSent);
    WATCH(numReceived);

    // addressing
    localPort = par("localPort").intValue();
    destPort = par("destPort").intValue();
    startTime = par("startTime").doubleValue();
    stopTime = par("stopTime").doubleValue();
    // packet and IP options
    packetName = par("packetName");
    dontFragment = par("dontFragment").boolValue();

    if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
      throw cRuntimeError("Invalid startTime/stopTime parameters");
    selfMsgAppTimer = new cMessage("applicationTimer");
    selfMsgSendTimer = new cMessage("sendTimer");
  }
}

void BaseApp::finish() {
  recordScalar("packets sent", numSent);
  recordScalar("packets received", numReceived);
  ApplicationBase::finish();
}

void BaseApp::refreshDisplay() const {
  ApplicationBase::refreshDisplay();

  char buf[100];
  sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
  getDisplayString().setTagArg("t", 0, buf);
}

void BaseApp::scheduleNextSendEvent(simtime_t time) {
  simtime_t base = time < 0.0 ? omnetpp::simTime() : time;
  simtime_t nextSend = base + par("sendInterval");

  if (nextSend <= stopTime) {
    if (selfMsgSendTimer->isScheduled()) {
      throw cRuntimeError("Cannot reschedule selfMsgSendTimer message.");
    }
    selfMsgSendTimer->setKind(FsmRootStates::SEND);
    scheduleAt(nextSend, selfMsgSendTimer);
  } else {
    EV_INFO << "Next send time after stopTime. Do not schedule new send event."
            << endl;
  }
}

void BaseApp::sendPayload(IntrusivePtr<const ApplicationPacket> payload) {
  L3Address destAddr = chooseDestAddr();
  sendPayload(payload, destAddr, destPort);
}

void BaseApp::sendPayload(IntrusivePtr<const ApplicationPacket> payload,
                          L3Address destAddr, int destPort) {
  std::ostringstream str;
  str << packetName << "-" << getId() << "#" << numSent;
  Packet *packet = new Packet(str.str().c_str());
  if (dontFragment) packet->addTag<FragmentationReq>()->setDontFragment(true);
  packet->insertAtBack(payload);
  emit(packetSentSignal, packet);
  EV_TRACE << "send packet: " << str.str().c_str()
           << " to destAddr: " << destAddr << " destPort: " << destPort << endl;
  sendToSocket(packet, destAddr, destPort);
  numSent++;
}

L3Address BaseApp::chooseDestAddr() {
  int k = intrand(destAddresses.size());
  if (destAddresses[k].isUnspecified() || destAddresses[k].isLinkLocal()) {
    L3AddressResolver().tryResolve(destAddressStr[k].c_str(), destAddresses[k]);
  }
  return destAddresses[k];
}

// Lifecycle management

void BaseApp::handleStartOperation(LifecycleOperation *operation) {
  simtime_t start = std::max(startTime, simTime());
  if ((stopTime < SIMTIME_ZERO) || (start < stopTime) ||
      (start == stopTime && startTime == stopTime)) {
    selfMsgAppTimer->setKind(FsmRootStates::SETUP);
    scheduleAt(start, selfMsgAppTimer);
  }
}

void BaseApp::handleStopOperation(LifecycleOperation *operation) {
  cancelEvent(selfMsgAppTimer);
  selfMsgAppTimer->setKind(FsmRootStates::TEARDOWN);
  handleMessageWhenUp(selfMsgAppTimer);
}

void BaseApp::handleCrashOperation(LifecycleOperation *operation) {
  cancelEvent(selfMsgAppTimer);
  selfMsgAppTimer->setKind(FsmRootStates::DESTROY);
  handleMessageWhenUp(selfMsgAppTimer);
}

/**
 * The root Finite State Machine handles basic lifecycle operations and
 * delegate selfMessage and Packet handling to fsmHandleSelfMsg and
 * fsmHandleWithSocket
 */
void BaseApp::handleMessageWhenUp(cMessage *msg) {
  socketFsmResult = FsmRootStates::ERR;
  FSM_Switch(fsmRoot) {
    // Init state ...
    case FSM_Exit(FsmRootStates::INIT):
      FSM_Goto(fsmRoot, FsmRootStates::SETUP);
      break;
    //
    // FSM_Steady
    case FSM_Exit(FsmRootStates::WAIT_ACTIVE):
      if (msg->isSelfMessage()) {
        FsmState state = msg->getKind();
        if (std::abs(state) >= FsmRootStates::SUBSTATE) {
          // Handle sub states in fsmHandleSelfMsg
          FSM_Goto(fsmRoot, fsmHandleSelfMsg(msg));
        } else {
          FSM_Goto(fsmRoot, msg->getKind());
        }
      } else {
        // received message is a packet. Let corresponding socket handle this.
        // This call will set socketFsmResult
        getSocket().processMessage(msg);
        FSM_Goto(fsmRoot, socketFsmResult);
      }
      break;
    case FSM_Exit(FsmRootStates::WAIT_INACTIVE):
      throw cRuntimeError("Application stop time reached! WAIT_INACTIVE");
      break;
    case FSM_Enter(FsmRootStates::ERR):  // error! if state is entered
      throw cRuntimeError("Reached Error state");
      break;
    //
    // FSM_Transient
    case FSM_Exit(FsmRootStates::SETUP):
      FSM_Goto(fsmRoot, fsmSetup(msg));
      break;
    case FSM_Exit(FsmRootStates::SEND):
      FSM_Goto(fsmRoot, fsmSend(msg));
      break;
    case FSM_Exit(FsmRootStates::TEARDOWN):
      FSM_Goto(fsmRoot, fsmTeardown(msg));
      break;
    case FSM_Exit(FsmRootStates::DESTROY):
      FSM_Goto(fsmRoot, fsmDestroy(msg));
      break;
  }
}

// FSM

BaseApp::FsmState BaseApp::fsmSetup(cMessage *msg) {
  const char *destAddrs = par("destAddresses");
  cStringTokenizer tokenizer(destAddrs);
  const char *token;

  while ((token = tokenizer.nextToken()) != nullptr) {
    destAddressStr.push_back(token);
    L3Address result;
    L3AddressResolver().tryResolve(token, result);
    if (result.isUnspecified())
      EV_ERROR << "cannot resolve destination address: " << token << endl;
    destAddresses.push_back(result);
  }

  initSocket();

  if (stopTime >= SIMTIME_ZERO) {
    selfMsgAppTimer->setKind(FsmRootStates::TEARDOWN);
    scheduleAt(stopTime, selfMsgAppTimer);
  }

  if (destAddresses.empty()) {
    EV_WARN << "no destination address found. Module will not send packets"
            << endl;
  } else {
    // schedule at startTime or current time, whatever is bigger.
    scheduleNextSendEvent(std::max(startTime, simTime()));
  }
  return FsmRootStates::WAIT_ACTIVE;
}

BaseApp::FsmState BaseApp::fsmTeardown(cMessage *msg) {
  getSocket().close();
  delayActiveOperationFinish(par("stopOperationTimeout"));
  return FsmRootStates::WAIT_INACTIVE;
}

BaseApp::FsmState BaseApp::fsmDestroy(cMessage *msg) {
  getSocket().destroy();
  // TODO  in real operating systems, program crash detected
  // by OS and OS closes sockets of crashed programs.
  return FsmRootStates::WAIT_INACTIVE;
}

}  // namespace rover
