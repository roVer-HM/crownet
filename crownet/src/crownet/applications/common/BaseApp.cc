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

namespace crownet {
BaseApp::~BaseApp() {
  if (appLifeTime) cancelAndDelete(appLifeTime);
  if (appMainTimer) cancelAndDelete(appMainTimer);
}

void BaseApp::initialize(int stage) {
  ApplicationBase::initialize(stage);
  if (stage == INITSTAGE_LOCAL) {
    numSent = 0;
    numReceived = 0;
    WATCH(numSent);
    WATCH(numReceived);

    startTime = par("startTime").doubleValue();
    stopTime = par("stopTime").doubleValue();
    // packet and IP options
    packetName = par("packetName");
    dontFragment = par("dontFragment").boolValue();

    if (stopTime > SIMTIME_ZERO && stopTime < startTime)
      throw cRuntimeError("Invalid startTime/stopTime parameters");
    appLifeTime = new cMessage("applicationTimer");
    appMainTimer = new cMessage("sendTimer");

    socketProvider = inet::getModuleFromPar<SocketProvider>(par("socketModule"), this);
  }
}

void BaseApp::setFsmResult(const FsmState &state) { socketFsmResult = state; }

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

void BaseApp::scheduleNextAppMainEvent(simtime_t time) {
  if (par("appMainInterval").doubleValue() < 0.0) {
    EV_INFO << "AppMain processing deactivated" << endl;
    return;
  }
  simtime_t base = time < 0.0 ? omnetpp::simTime() : time;
  simtime_t nextSend = base + par("appMainInterval") + par ("appMainIntervalJitter");

  if ((stopTime == SIMTIME_ZERO) || (nextSend < stopTime)) {
    if (appMainTimer->isScheduled()) {
      throw cRuntimeError("Cannot reschedule selfMsgSendTimer message.");
    }
    appMainTimer->setKind(FsmRootStates::APP_MAIN);
    scheduleAt(nextSend, appMainTimer);
  } else {
    EV_INFO << "Next send time after stopTime. Do not schedule new send event."
            << endl;
  }
}

void BaseApp::cancelAppMainEvent() {
  if (appMainTimer->isScheduled()) cancelEvent(appMainTimer);
}

void BaseApp::sendPayload(IntrusivePtr<const ApplicationPacket> payload) {
    std::ostringstream str;
    str << packetName << "-" << getId() << "#" << numSent;
    Packet *packet = new Packet(str.str().c_str());
    if (dontFragment) packet->addTag<FragmentationReq>()->setDontFragment(true);
    packet->insertAtBack(payload);
    emit(packetSentSignal, packet);
    numSent++;
    send(packet, gate("socketOut"));
}




// Lifecycle management

void BaseApp::handleStartOperation(LifecycleOperation *operation) {
  simtime_t start = std::max(startTime, simTime());
  if ((stopTime == SIMTIME_ZERO) || (start <= stopTime) ||
      (start == stopTime && startTime == stopTime)) {
    appLifeTime->setKind(FsmRootStates::SETUP);
    scheduleAt(start, appLifeTime);
  } else {
    EV << "Lifecycle not started for BaseApp" << endl;
  }
}

void BaseApp::handleStopOperation(LifecycleOperation *operation) {
  cancelEvent(appLifeTime);
  appLifeTime->setKind(FsmRootStates::TEARDOWN);
  handleMessageWhenUp(appLifeTime);
}

void BaseApp::handleCrashOperation(LifecycleOperation *operation) {
  cancelEvent(appLifeTime);
  appLifeTime->setKind(FsmRootStates::DESTROY);
  handleMessageWhenUp(appLifeTime);
}

/**
 * The root Finite State Machine handles basic lifecycle operations and
 * delegate selfMessage and Packet handling to fsmHandleSelfMsg and
 * fsmHandleWithSocket
 */
void BaseApp::handleMessageWhenUp(cMessage *msg) {
  socketFsmResult = FsmRootStates::ERR;
  // fsmRoot == current state
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
      } else if (msg->arrivedOn("socketIn")) {
          FSM_Goto(fsmRoot, fsmDataArrived(msg));
      } else {
          throw cRuntimeError("Unkonwn Packet");
      }
      break;
    case FSM_Exit(FsmRootStates::WAIT_INACTIVE):
      EV_INFO << "Application stop time reached! WAIT_INACTIVE" << std::endl;
//      throw cRuntimeError("Application stop time reached! WAIT_INACTIVE");
      break;
    case FSM_Enter(FsmRootStates::ERR):  // error! if state is entered
      throw cRuntimeError("Reached Error state");
      break;
    //
    // FSM_Transient
    case FSM_Exit(FsmRootStates::SETUP):
      FSM_Goto(fsmRoot, fsmSetup(msg));
      break;
    case FSM_Exit(FsmRootStates::APP_MAIN):
      FSM_Goto(fsmRoot, fsmAppMain(msg));
      break;
    case FSM_Exit(FsmRootStates::TEARDOWN):
      FSM_Goto(fsmRoot, fsmTeardown(msg));
      break;
    case FSM_Exit(FsmRootStates::DESTROY):
      FSM_Goto(fsmRoot, fsmDestroy(msg));
      break;
  }
}

void BaseApp::setupTimers() {
  if (socketProvider->hasDestAddress()) {
    EV_WARN << "no destination address found. Module will not send packets"
            << endl;
  } else {
    // schedule at startTime or current time, whatever is bigger.
    scheduleNextAppMainEvent(std::max(startTime, simTime()));
  }
}

// FSM

FsmState BaseApp::fsmHandleSelfMsg(cMessage *msg) {
    throw cRuntimeError("Mo selfMsg on BaseApp");
}

FsmState BaseApp::fsmSetup(cMessage *msg) {
  socketProvider->setupSocket();

  // todo check msg == appLifeTime (may lead to reschedule without)
  if (stopTime > SIMTIME_ZERO) {
    appLifeTime->setKind(FsmRootStates::TEARDOWN);
    scheduleAt(stopTime, appLifeTime);
  }

  setupTimers();
  return FsmRootStates::WAIT_ACTIVE;
}

FsmState BaseApp::fsmDataArrived(cMessage *msg){
    numReceived++;
    Packet* pk = check_and_cast<Packet *>(msg);
    emit(packetReceivedSignal, pk);
    FsmState next = handleDataArrived(pk);
    delete pk;
    return next;
}

FsmState BaseApp::fsmAppMain(cMessage *msg) {
  // do nothing just reschedule
  scheduleNextAppMainEvent();
  return FsmRootStates::WAIT_ACTIVE;
}

FsmState BaseApp::fsmTeardown(cMessage *msg) {
    socketProvider->close();
//  delayActiveOperationFinish(par("stopOperationTimeout"));    // todo: correctly  implement ILifecycle ...
  cancelAndDelete(appLifeTime);
  appLifeTime = nullptr;
  cancelAndDelete(appMainTimer);
  appMainTimer = nullptr;

  return FsmRootStates::WAIT_INACTIVE;
}

FsmState BaseApp::fsmDestroy(cMessage *msg) {
    socketProvider->destroy();
  // TODO  in real operating systems, program crash detected
  // by OS and OS closes sockets of crashed programs.
  cancelAndDelete(appLifeTime);
  appLifeTime = nullptr;
  cancelAndDelete(appMainTimer);
  appMainTimer = nullptr;

  return FsmRootStates::WAIT_INACTIVE;
}

}  // namespace crownet
