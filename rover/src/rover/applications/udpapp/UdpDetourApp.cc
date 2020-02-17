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
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "rover/applications/udpapp/DetourAppPacket_m.h"

using namespace inet;
using omnetpp::cStringTokenizer;

namespace rover {
Define_Module(UdpDetourApp);

UdpDetourApp::~UdpDetourApp() { cancelAndDelete(selfMsgIncident); }

void UdpDetourApp::initialize(int stage) {
  UdpBasicApp::initialize(stage);

  if (stage == INITSTAGE_LOCAL) {
    reason = par("reason").stdstringValue();
    closedTarget = par("closedTarget").intValue();
    repeatTime = par("repeatTime").doubleValue();
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

    selfMsgIncident = new cMessage("incidentTimer");
    simtime_t incidentTime = par("incidentTime").doubleValue();
    if (incidentTime != -1.0) {
      selfMsgIncident->setKind(SEND);
      scheduleAt(incidentTime, selfMsgIncident);
    }
  }
}

void UdpDetourApp::handleMessageWhenUp(cMessage *msg) {
  if (msg->isSelfMessage()) {
    ASSERT(msg == selfMsg | msg == selfMsgIncident);
    switch (msg->getKind()) {
      case PROPAGATE:
        processPropagate();
        break;
      case START:
        processStart();
        break;

      case SEND:
        processSend();
        break;

      case STOP:
        processStop();
        break;

      default:
        throw cRuntimeError("Invalid kind %d in self message",
                            (int)selfMsg->getKind());
    }
  } else
    socket.processMessage(msg);
}

// send incident packet.
void UdpDetourApp::sendPacket() {
  std::ostringstream str;
  str << packetName << "-" << numSent;
  Packet *packet = new Packet(str.str().c_str());
  if (dontFragment) packet->addTag<FragmentationReq>()->setDontFragment(true);
  const auto &payload = makeShared<DetourAppPacket>();
  // application data
  payload->setReason(reason.c_str());
  payload->setEventTime(simTime());
  payload->setClosedTarget(closedTarget);
  payload->setAlternativeRouteArraySize(alternativeRoute.size());
  payload->setRepeatTime(repeatTime);
  payload->setRepeateInterval(par("repeateInterval").doubleValue());
  for (int i = 0; i < alternativeRoute.size(); i++) {
    payload->setAlternativeRoute(i, alternativeRoute.at(i));
  }
  // application data end
  //  payload->setChunkLength(B(par("messageLength")));
  payload->setSequenceNumber(numSent);
  payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
  payload->setChunkLength(B(par("messageLength")));
  packet->insertAtBack(payload);
  L3Address destAddr = chooseDestAddr();
  emit(packetSentSignal, packet);
  socket.sendTo(packet, destAddr, destPort);
  numSent++;
}

void UdpDetourApp::processStart() {
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
      selfMsg->setKind(STOP);
      scheduleAt(stopTime, selfMsg);
    }
  }
}

void UdpDetourApp::processSend() { UdpBasicApp::processSend(); }

void UdpDetourApp::processPacket(Packet *pk) {
  // todo schedule propagate
  // todo send traci command
  UdpBasicApp::processPacket(pk);
}

void UdpDetourApp::processPropagate() {
  // todo handle propagate
}

} /* namespace rover */
