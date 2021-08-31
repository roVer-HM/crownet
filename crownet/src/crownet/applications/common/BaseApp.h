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
#include "crownet/applications/common/AppCommon_m.h"

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
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"

#include "crownet/applications/common/info/AppInfoLocal.h"

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
  simtime_t appMainIntervalOffset;
  int sendLimit;

  const char *packetName = nullptr;

  // state
  cMessage *appLifeTime = nullptr;
  cMessage *appMainTimer = nullptr;

  omnetpp::cFSM fsmRoot;
  FsmState socketFsmResult = FsmRootStates::ERR;
  SocketProvider* socketProvider;
  AppInfoLocal* localInfo = nullptr;

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
  // use simple ApplicationPacket based application.
  // No separate header, footer
  virtual void sendPayload(IntrusivePtr<ApplicationPacket> payload);
  virtual void sendPayload(IntrusivePtr<ApplicationPacket> payload, L3Address addr, int port);
  // let application create complex packet structure. BaseApp child classes must ensure correct
  // sequence number and tags.
  virtual void sendPayload(Packet *packet);
  virtual void sendPayload(Packet *packet,  L3Address addr, int port);

  virtual AppInfoLocal* createLocalAppInfo();

  template <typename T>
  IntrusivePtr<T> createPacket(b length = b(-1));

  // fsmRoot actions

  virtual void setFsmResult(const FsmState &state);

  // handle extra self SubState
  virtual FsmState fsmHandleSubState(cMessage *msg);
  // setup socket, endTime and selfMsgSendTimer
  virtual FsmState fsmSetup(cMessage *msg);
  virtual FsmState fsmDataArrived(cMessage *msg);
  virtual FsmState fsmAppMain(cMessage *msg) = 0;
  virtual FsmState fsmTeardown(cMessage *msg);
  virtual FsmState fsmDestroy(cMessage *msg);


  virtual void setupTimers();  // called in fsmSetup
  simtime_t getInitialMainAppTime();

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
  payload->setSequenceNumber(localInfo->nextSequenceNumber());
  payload->template addTag<CreationTimeTag>()->setCreationTime(simTime());
  return payload;
}

}  // namespace crownet
