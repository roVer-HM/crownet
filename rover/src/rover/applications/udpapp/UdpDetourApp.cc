/*
 * UdpDetourApp.cpp
 *
 *  Created on: Feb 12, 2020
 *      Author: sts
 */

#include "UdpDetourApp.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"
#include "rover/applications/udpapp/DetourAppPacket_m.h"

using namespace inet;
using omnetpp::cStringTokenizer;

namespace rover {
Define_Module(UdpDetourApp);

UdpDetourApp::~UdpDetourApp() {
  cancelAndDelete(selfMsg);
  cancelAndDelete(selfMsgIncident);
  for (auto &item : handlerMap) {
    cancelAndDelete(item.second.msg);
  }
  // todo delete cMessages in PropagationHandle
}

void UdpDetourApp::initialize(int stage) {
  ApplicationBase::initialize(stage);

  if (stage == INITSTAGE_LOCAL) {
    // Statistics
    numSent = 0;
    numReceived = 0;
    WATCH(numSent);
    WATCH(numReceived);
    // UdpBasicApp
    localPort = par("localPort");
    destPort = par("destPort");
    startTime = par("startTime");
    stopTime = par("stopTime");
    packetName = par("packetName");
    dontFragment = par("dontFragment");
    // DetoureApp
    incidentTime = par("incidentTime").doubleValue();
    reason = par("reason").stdstringValue();
    closedTarget = par("closedTarget").intValue();
    repeatTime = par("repeatTime").doubleValue();
    notifyMobilityProvider = par("notifyMobilityProvider").boolValue();
    cStringTokenizer tokenizer(par("alternativeRoute").stringValue(), ",");
    const char *token;
    while ((token = tokenizer.nextToken()) != nullptr) {
      try {
        alternativeRoute.push_back(std::stoi(token));
      } catch (const std::exception &e) {
        EV_ERROR << "cannot parse alternativeRoute parameter: "
                 << "alternativeRoute" << std::endl;
        throw cRuntimeError("cannot parse alternativeRoute parameter: %s",
                            par("alternativeRoute").stringValue());
      }
    }
    // ensure sender is correctly configured.
    if (incidentTime > SIMTIME_ZERO &&
        (reason.empty() || closedTarget == -1 || alternativeRoute.size() == 0 ||
         repeatTime <= SIMTIME_ZERO ||
         (simtime_t)(par("repeateInterval").doubleValue()) <= SIMTIME_ZERO)) {
      throw cRuntimeError(
          "UdpDetorApp(sender config) must provide a reason, closedTarget and "
          "alternativeRoute. Some of these parameters are not set correctly");
    }

    // application life cycle (selfMsg)
    if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
      throw cRuntimeError("Invalid startTime/stopTime parameters");
    selfMsg = new cMessage("applicationTimer");

    // source of information (selfMsg)
    selfMsgIncident = new cMessage("incidentTimer");
  }
}

void UdpDetourApp::handleMessageWhenUp(cMessage *msg) {
  nextState = FsmStates::ERR;  // fallback to error state;
  FSM_Switch(fsm) {
    // Init state ...
    case FSM_Exit(FsmStates::INIT):
      FSM_Goto(fsm, FsmStates::SETUP);
      break;
    //
    // FSM_Steady
    case FSM_Exit(FsmStates::WAIT_ACTIVE):
      if (msg->isSelfMessage()) {
        FSM_Goto(fsm, msg->getKind());  // use MsgKind to determine next state
      } else {
        socket.processMessage(msg);  // Callback will set nextState
        FSM_Goto(fsm, nextState);
      }
      break;
    case FSM_Exit(FsmStates::WAIT_INACTIVE):
      throw cRuntimeError("Application stop time reached! WAIT_INACTIVE");
      break;
    case FSM_Enter(FsmStates::ERR):  // error! if state is entered
      throw cRuntimeError("Reached Error state");
      break;
    //
    // FSM_Transient
    case FSM_Exit(FsmStates::SETUP):
      nextState = fsmSetupExit(msg);
      FSM_Goto(fsm, nextState);
      break;
    case FSM_Exit(FsmStates::TEARDOWN):
      nextState = fsmTeardownExit(msg);
      FSM_Goto(fsm, nextState);
      break;
    case FSM_Exit(FsmStates::INCIDENT_TX):
      nextState = fsmIncidentTxExit(msg);
      FSM_Goto(fsm, nextState);
      break;
    case FSM_Exit(FsmStates::PROPAGATE_TX):
      nextState = fsmPropagateTxExit(msg);
      FSM_Goto(fsm, nextState);
      break;
    case FSM_Exit(FsmStates::DESTROY):
      nextState = fsmDestroyExit(msg);
      FSM_Goto(fsm, nextState);
  }
}

void UdpDetourApp::finish() {
  recordScalar("packets sent", numSent);
  recordScalar("packets received", numReceived);
  ApplicationBase::finish();
}

void UdpDetourApp::refreshDisplay() const {
  ApplicationBase::refreshDisplay();

  char buf[100];
  sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
  getDisplayString().setTagArg("t", 0, buf);
}

void UdpDetourApp::setSocketOptions() {
  int timeToLive = par("timeToLive");
  if (timeToLive != -1) socket.setTimeToLive(timeToLive);

  int dscp = par("dscp");
  if (dscp != -1) socket.setDscp(dscp);

  int tos = par("tos");
  if (tos != -1) socket.setTos(tos);

  const char *multicastInterface = par("multicastInterface");
  if (multicastInterface[0]) {
    IInterfaceTable *ift =
        getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    InterfaceEntry *ie = ift->findInterfaceByName(multicastInterface);
    if (!ie)
      throw cRuntimeError(
          "Wrong multicastInterface setting: no interface named \"%s\"",
          multicastInterface);
    socket.setMulticastOutputInterface(ie->getInterfaceId());
  }

  bool receiveBroadcast = par("receiveBroadcast");
  if (receiveBroadcast) socket.setBroadcast(true);

  bool joinLocalMulticastGroups = par("joinLocalMulticastGroups");
  if (joinLocalMulticastGroups) {
    MulticastGroupList mgl =
        getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)
            ->collectMulticastGroups();
    socket.joinLocalMulticastGroups(mgl);
  }
  socket.setCallback(this);
}

void UdpDetourApp::handleStartOperation(LifecycleOperation *operation) {
  // start application
  simtime_t start = std::max(startTime, simTime());
  if ((stopTime < SIMTIME_ZERO) || (start < stopTime) ||
      (start == stopTime && startTime == stopTime)) {
    selfMsg->setKind(FsmStates::SETUP);
    scheduleAt(start, selfMsg);
  }

  // start incidentTimer if node is the source of the information.
  if (isSender()) {
    selfMsgIncident->setKind(FsmStates::INCIDENT_TX);
    scheduleAt(incidentTime, selfMsgIncident);
  }
}

void UdpDetourApp::handleStopOperation(LifecycleOperation *operation) {
  cancelEvent(selfMsg);
  selfMsg->setKind(FsmStates::TEARDOWN);
  handleMessageWhenUp(selfMsg);
}

void UdpDetourApp::handleCrashOperation(LifecycleOperation *operation) {
  cancelEvent(selfMsg);
  selfMsg->setKind(FsmStates::DESTROY);
  handleMessageWhenUp(selfMsg);
}

void UdpDetourApp::socketDataArrived(UdpSocket *socket, Packet *packet) {
  IntrusivePtr<const DetourAppPacket> pkt =
      packet->popAtFront<DetourAppPacket>();

  if (pkt->getType() == DetourPktType::INCIDENT) {
    nextState = fsmIncidentRxExit(pkt);
  } else if (pkt->getType() == DetourPktType::PROPAGATE) {
    nextState = fsmPropagateRxExit(pkt);
  } else {
    throw new cRuntimeError("DetorTags not found");
  }
}

void UdpDetourApp::socketErrorArrived(UdpSocket *socket,
                                      Indication *indication) {
  EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
  delete indication;
  nextState = FsmStates::WAIT_ACTIVE;  // ignore
}

void UdpDetourApp::socketClosed(UdpSocket *socket) {
  if (operationalState == State::STOPPING_OPERATION) {
    startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
  }
  nextState = FsmStates::TEARDOWN;
}

Coord UdpDetourApp::getCurrentLocation() {
  if (!mobilityModule) {
    mobilityModule = check_and_cast<MobilityBase *>(
        getParentModule()->getSubmodule("mobility"));
  }
  return mobilityModule->getCurrentPosition();
}

const bool UdpDetourApp::isSender() { return incidentTime > 0.0; }

void UdpDetourApp::actOnIncident(IntrusivePtr<const DetourAppPacket> pkt) {
  // check if Incident id meant for me (default yes)
}

void UdpDetourApp::sendPayload(IntrusivePtr<const DetourAppPacket> payload) {
  std::ostringstream str;
  str << payload->getIncidentReason() << "-" << numSent;
  Packet *packet = new Packet(str.str().c_str());
  if (dontFragment) packet->addTag<FragmentationReq>()->setDontFragment(true);
  packet->insertAtBack(payload);
  L3Address destAddr = destAddresses[0];
  emit(packetSentSignal, packet);
  socket.sendTo(packet, destAddr, destPort);
  numSent++;
}

// FSM

UdpDetourApp::FsmStates UdpDetourApp::fsmTeardownExit(cMessage *msg) {
  socket.close();
  delayActiveOperationFinish(par("stopOperationTimeout"));
  return FsmStates::WAIT_INACTIVE;
}

UdpDetourApp::FsmStates UdpDetourApp::fsmDestroyExit(cMessage *msg) {
  socket.destroy();  // TODO  in real operating systems, program crash detected
                     // by OS and OS closes sockets of crashed programs.
  return FsmStates::WAIT_INACTIVE;
}

UdpDetourApp::FsmStates UdpDetourApp::fsmSetupExit(cMessage *msg) {
  socket.setOutputGate(gate("socketOut"));
  const char *localAddress = par("localAddress");
  socket.bind(
      *localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(),
      localPort);
  setSocketOptions();

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

  if (destAddresses.empty()) {
    if (stopTime >= SIMTIME_ZERO) {
      selfMsg->setKind(FsmStates::TEARDOWN);
      scheduleAt(stopTime, selfMsg);
    }
  }
  return FsmStates::WAIT_ACTIVE;
}

// send incident packet.
UdpDetourApp::FsmStates UdpDetourApp::fsmIncidentTxExit(cMessage *msg) {
  const auto &payload = makeShared<DetourAppPacket>();
  // application data
  payload->setIncidentReason(reason.c_str());
  payload->setIncidentTime(simTime());
  payload->setClosedTarget(closedTarget);
  payload->setAlternativeRouteArraySize(alternativeRoute.size());
  payload->setRepeatTime(repeatTime);
  payload->setRepeateInterval(par("repeateInterval").doubleValue());
  payload->setIncidentOrigin(getCurrentLocation());
  payload->setLastHopOrigin(getCurrentLocation());
  payload->setType(DetourPktType::INCIDENT);
  for (int i = 0; i < alternativeRoute.size(); i++) {
    payload->setAlternativeRoute(i, alternativeRoute.at(i));
  }
  payload->setSequenceNumber(numSent);
  payload->setChunkLength(B(par("messageLength")));

  std::ostringstream str;
  str << payload->getIncidentReason() << "-" << numSent;
  Packet *packet = new Packet(str.str().c_str());
  if (dontFragment) packet->addTag<FragmentationReq>()->setDontFragment(true);
  packet->insertAtBack(payload);
  L3Address destAddr = destAddresses[0];
  emit(packetSentSignal, packet);

  socket.sendTo(packet, destAddr, destPort);
  numSent++;

  //  sendPayload(payload);

  return FsmStates::WAIT_ACTIVE;
}

UdpDetourApp::FsmStates UdpDetourApp::fsmIncidentRxExit(
    IntrusivePtr<const DetourAppPacket> pkt) {
  // do I know this incident already?
  PropagationHandleMap::iterator it = handlerMap.find(pkt->getIncidentReason());
  if (it != handlerMap.end()) {
    // reason is known. for now do nothing

  } else {
    // reason is new
    actOnIncident(pkt);

    // Prepare Propagation
    cMessage *msg = new cMessage(pkt->getIncidentReason());
    PropagationHandle newHandle{pkt, msg, pkt->getRepeatTime()};

    msg->setKind(FsmStates::PROPAGATE_TX);
    newHandle.restTime -= newHandle.pkt->getRepeateInterval();
    simtime_t nextEvent = simTime() + newHandle.pkt->getRepeateInterval();
    ASSERT(nextEvent > simTime());
    scheduleAt(nextEvent, msg);

    handlerMap.insert(std::pair<std::string, PropagationHandle>(
        pkt->getIncidentReason(), newHandle));
  }
  return FsmStates::WAIT_ACTIVE;
}

UdpDetourApp::FsmStates UdpDetourApp::fsmPropagateTxExit(cMessage *msg) {
  PropagationHandleMap::iterator it = handlerMap.find(msg->getName());
  if (it != handlerMap.end()) {
    // Propagate message
    ASSERT(msg == it->second.msg);
    auto payload = IntrusivePtr<DetourAppPacket>(it->second.pkt->dup());
    payload->setLastHopOrigin(getCurrentLocation());
    payload->setSequenceNumber(numSent);
    payload->setType(DetourPktType::PROPAGATE);
    sendPayload(payload);

    // schedule next if time budget is there.
    it->second.restTime -= it->second.pkt->getRepeateInterval();
    if (it->second.restTime > 0.0) {
      simtime_t nextEvent = simTime() + it->second.pkt->getRepeateInterval();
      ASSERT(nextEvent > simTime());
      scheduleAt(nextEvent, it->second.msg);
    } else {
      // budget is gone. cancelEvent but do not delete it (may be needed later
      // on). do not remove it PropagationHandle from map so new messages will
      // not be Registered as new once.
      cancelEvent(msg);
    }
  } else {
    throw cRuntimeError(
        "PropagateTxExit for reason: %s was scheduled but PropagationHandle "
        "does not exist. Some SelfMessage not canceled?",
        msg->getName());
  }
  return FsmStates::WAIT_ACTIVE;
}

UdpDetourApp::FsmStates UdpDetourApp::fsmPropagateRxExit(
    IntrusivePtr<const DetourAppPacket> pkt) {
  // for now treat a Propagation Packet the same way as an Incided.
  // Meaning: act on incident if not seen before and setup propagation.
  return fsmIncidentRxExit(pkt);
}

} /* namespace rover */
