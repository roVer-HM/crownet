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
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

using namespace inet;

namespace crownet {
BaseApp::~BaseApp() {
  if (appLifeTime) cancelAndDelete(appLifeTime);
  if (appMainTimer) cancelAndDelete(appMainTimer);
  delete localInfo;
}

void BaseApp::initialize(int stage) {
  ApplicationBase::initialize(stage);
  if (stage == INITSTAGE_LOCAL) {

    startTime = par("startTime").doubleValue();
    stopTime = par("stopTime").doubleValue();
    appMainIntervalOffset = par("mainIntervalOffset").doubleValue();
    sendLimit = par("mainMsgLimit").intValue();

    auto info = createLocalAppInfo();
    info->setWalltimeStart(startTime);
    localInfo = info;
    take(localInfo);


    if (appMainIntervalOffset < 0.){
        throw cRuntimeError("Invalid mainIntervalOffset parameters must be >= 0");
    }
    // packet and IP options
    packetName = par("packetName");


    if (stopTime > SIMTIME_ZERO && stopTime < startTime){
      throw cRuntimeError("Invalid startTime/stopTime parameters");
    }
    appLifeTime = new cMessage("applicationTimer");
    appMainTimer = new cMessage("sendTimer");

    socketProvider = inet::getModuleFromPar<SocketProvider>(par("socketModule"), this);
  }
}

AppInfoLocal* BaseApp::createLocalAppInfo(){
    // todo: select AppInfoType based on config
    auto appInfo = new AppInfoLocal();

    appInfo->setNodeId(getId());
    appInfo->setSequencenumber(0);
    return appInfo;
}

void BaseApp::setFsmResult(const FsmState &state) { socketFsmResult = state; }

void BaseApp::finish() {
  recordScalar("packets sent", localInfo->getPacketsSentCount());
  recordScalar("packets received", localInfo->getPacketsReceivedCount());
  ApplicationBase::finish();
}

void BaseApp::refreshDisplay() const {
  ApplicationBase::refreshDisplay();

  char buf[100];
  sprintf(buf, "rcvd: %d pks\nsent: %d pks", localInfo->getPacketsReceivedCount(), localInfo->getPacketsSentCount());
  getDisplayString().setTagArg("t", 0, buf);
}

void BaseApp::scheduleNextAppMainEvent(simtime_t time) {
  if (par("mainInterval").doubleValue() < 0.0) {
    EV_INFO << "AppMain processing deactivated" << endl;
    return;
  }
  simtime_t nextSend;
  if (time < SIMTIME_ZERO){
      nextSend = omnetpp::simTime() + par("mainInterval") + par ("mainIntervalJitter");
  } else {
      nextSend = time;
  }

  if ((stopTime == SIMTIME_ZERO) || (nextSend < stopTime)) {
    if (appMainTimer->isScheduled()) {
      throw cRuntimeError("Cannot reschedule selfMsgSendTimer message.");
    }
    if (sendLimit !=0){
        // schedule if sendLimit is not reached  >0 or if sendLimit is deactivated <0
        appMainTimer->setKind(FsmRootStates::APP_MAIN);
        scheduleAt(nextSend, appMainTimer);
    } else {
        EV_INFO << "Message limit reached." << endl;
    }
  } else {
    EV_INFO << "Next send time after stopTime. Do not schedule new send event."
            << endl;
  }
}

void BaseApp::cancelAppMainEvent() {
  if (appMainTimer->isScheduled()) cancelEvent(appMainTimer);
}

/**
 * use default address and port configured in socket
 */
void BaseApp::sendPayload(IntrusivePtr<ApplicationPacket> payload) {
    Packet *packet = new Packet(localInfo->packetName(packetName).c_str());
    packet->insertAtBack(payload);
    sendPayload(packet);
}

/**
 * use application logic dependent address and port and override socket default
 */
void BaseApp::sendPayload(IntrusivePtr<ApplicationPacket> payload, L3Address addr, int port){
    Packet *packet = new Packet(localInfo->packetName(packetName).c_str());
    packet->insertAtBack(payload);
    sendPayload(packet, addr, port);
}

void BaseApp::sendPayload(Packet *packet){
    emit(packetSentSignal, packet);
    localInfo->incrSent();
    if (sendLimit > 0){
        sendLimit--;
    }
    send(packet, gate("socketOut"));
}

void BaseApp::sendPayload(Packet *packet,  L3Address addr, int port){
    packet->addTagIfAbsent<L3AddressReq>()->setDestAddress(addr);
    packet->addTagIfAbsent<L4PortReq>()->setDestPort(port);
    emit(packetSentSignal, packet);
    localInfo->incrSent();
    if (sendLimit > 0){
        sendLimit--;
    }
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
          FSM_Goto(fsmRoot, fsmHandleSubState(msg));
        } else {
          FSM_Goto(fsmRoot, msg->getKind());
        }
      } else if (msg->arrivedOn("socketIn")) {
          FSM_Goto(fsmRoot, fsmDataArrived(msg));
      } else if (msg->arrivedOn("configIn")){
          // send config message to sub state handler
          FSM_Goto(fsmRoot, fsmHandleSubState(msg));
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
    // schedule first mainApp interval
    scheduleNextAppMainEvent(getInitialMainAppTime());
}

simtime_t BaseApp::getInitialMainAppTime(){
    return std::max(startTime, simTime()) + appMainIntervalOffset;
}


// FSM

FsmState BaseApp::fsmHandleSubState(cMessage *msg) {
    throw cRuntimeError("No Substate on BaseApp");
}

FsmState BaseApp::fsmSetup(cMessage *msg) {
  socketProvider->setupSocket();

  // todo check msg == appLifeTime (may lead to reschedule without)
  if (stopTime > SIMTIME_ZERO) {
    appLifeTime->setKind(FsmRootStates::TEARDOWN);
    scheduleAt(stopTime, appLifeTime);
  }

  if ( !par("allEmptyDestAddress").boolValue() && !socketProvider->hasDestAddress()) {
      cRuntimeError("No destination address found.");
  }

  setupTimers();
  return FsmRootStates::WAIT_ACTIVE;
}

FsmState BaseApp::fsmDataArrived(cMessage *msg){
    localInfo->incrReceivd();
    Packet* pk = check_and_cast<Packet *>(msg);
    emit(packetReceivedSignal, pk);
    FsmState next = handleDataArrived(pk);
    delete pk;
    return next;
}

FsmState BaseApp::fsmAppMain(cMessage *msg) {
  // do nothing
  EV_WARN << "BaseApp has no AppMain. Implement for active message generate. App will still receive messages!"
          << std::endl;
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
