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

#include <vector>

#include "crownet/common/result/Simsignals.h"
#include "crownet/applications/common/AppFsm.h"
#include "inet/common/INETDefs.h"
#include "inet/common/socket/ISocket.h"
#include "inet/networklayer/common/L3Address.h"

#include "inet/applications/base/ApplicationBase.h"

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"

using namespace inet;

namespace crownet {

class BaseApp : public ApplicationBase, public DataArrivedHandler {
 public:
  BaseApp(){};
  virtual ~BaseApp();

 public:
 protected:
  // parameters
  simtime_t startTime;
  simtime_t stopTime;

  const char *packetName = nullptr;
  bool dontFragment = false;

  // state
  cMessage *appLifeTime = nullptr;
  cMessage *appMainTimer = nullptr;

  // statistics
  int numSent = 0;
  int numReceived = 0;

  omnetpp::cFSM fsmRoot;
  FsmState socketFsmResult = FsmRootStates::ERR;
  SocketProvider* socketProvider;

 protected:
  virtual int numInitStages() const override { return NUM_INIT_STAGES; }
  virtual void handleMessageWhenUp(cMessage *msg) override;
  virtual void initialize(int stage) override;
  virtual void finish() override;
  virtual void refreshDisplay() const override;
  /**
   * schedule selfMsgSendTimer with base + par("sendInterval").
   * base = -1 defaults to the current time.
   * negative par("sendInterval") will cause errors.
   */
  virtual void scheduleNextAppMainEvent(simtime_t time = -1);
  virtual void cancelAppMainEvent();
  virtual void sendPayload(IntrusivePtr<const ApplicationPacket> payload);

  template <typename T>
  IntrusivePtr<T> createPacket(b length = b(-1));

  template <typename T>
  IntrusivePtr<const T> checkEmitGetReceived(Packet *pkt);

  // fsmRoot actions

  virtual void setFsmResult(const FsmState &state);

  // handle extra self Messages
  virtual FsmState fsmHandleSelfMsg(cMessage *msg);
  // setup socket, endTime and selfMsgSendTimer
  virtual FsmState fsmSetup(cMessage *msg);
  virtual FsmState fsmDataArrived(cMessage *msg);
  virtual FsmState fsmAppMain(cMessage *msg) = 0;
  virtual FsmState fsmTeardown(cMessage *msg);
  virtual FsmState fsmDestroy(cMessage *msg);


  virtual void setupTimers();  // called in fsmSetup

  // Lifecycle management
  virtual void handleStartOperation(
      LifecycleOperation *operation) override;  // trigger fsmSetup
  virtual void handleStopOperation(LifecycleOperation *operation) override;
  virtual void handleCrashOperation(LifecycleOperation *operation) override;
};

template <typename T>
inline IntrusivePtr<T> BaseApp::createPacket(b length) {
  auto payload = makeShared<T>();
  payload->setChunkLength(length);
  payload->setSequenceNumber(numSent);
  payload->template addTag<CreationTimeTag>()->setCreationTime(simTime());
  return payload;
}

template <typename T>
inline IntrusivePtr<const T> BaseApp::checkEmitGetReceived(Packet *pkt) {
  emit(packetReceivedSignal, pkt);
  numReceived++;

  return pkt->popAtFront<T>();
}

}  // namespace crownet
