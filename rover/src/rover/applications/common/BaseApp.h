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
#include "rover/common/Simsignals.h"

using namespace inet;

namespace rover {

class IRoverSocket : public ISocket {
 public:
  virtual ~IRoverSocket() {}
};

class BaseApp : public ApplicationBase {
 public:
  BaseApp(){};
  virtual ~BaseApp();

 public:
 protected:
  // parameters
  simtime_t startTime;
  simtime_t stopTime;

  int localPort = -1;
  int destPort = -1;
  std::vector<L3Address> destAddresses;
  std::vector<std::string> destAddressStr;
  const char *packetName = nullptr;
  bool dontFragment = false;

  // state
  cMessage *selfMsgAppTimer = nullptr;
  cMessage *selfMsgSendTimer = nullptr;

  // statistics
  int numSent = 0;
  int numReceived = 0;

  // Root Finite State Machine setup omnetpp::cFSM fsm;
  enum FsmRootStates {
    INIT = 0,
    WAIT_ACTIVE = FSM_Steady(1),
    WAIT_INACTIVE = FSM_Steady(2),
    ERR = FSM_Steady(3),
    SETUP = FSM_Transient(101),  // socket and stopTime
    APP_MAIN = FSM_Transient(
        102),  // send data with Interval or more elaborate functions
    TEARDOWN = FSM_Transient(110),
    DESTROY = FSM_Transient(120),
    SUBSTATE = 200  // sub states must be outside of +-200
  };
  typedef int FsmState;  // state for each FSM.
  omnetpp::cFSM fsmRoot;
  FsmState socketFsmResult = FsmRootStates::ERR;

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
  virtual void sendPayload(IntrusivePtr<const ApplicationPacket> payload);
  virtual void sendPayload(IntrusivePtr<const ApplicationPacket> payload,
                           L3Address destAddr, int destPort);

  virtual L3Address chooseDestAddr();

  // fsmRoot actions

  // handle extra self Messages
  virtual FsmState fsmHandleSelfMsg(cMessage *msg) = 0;
  // setup socket, endTime and selfMsgSendTimer
  virtual FsmState fsmSetup(cMessage *msg);
  virtual FsmState fsmAppMain(cMessage *msg) = 0;
  virtual FsmState fsmTeardown(cMessage *msg);
  virtual FsmState fsmDestroy(cMessage *msg);

  // socket actions.
  virtual void initSocket() = 0;
  virtual ISocket &getSocket() = 0;
  virtual void sendToSocket(Packet *msg, L3Address destAddr, int destPort) = 0;

  // Lifecycle management
  virtual void handleStartOperation(
      LifecycleOperation *operation) override;  // trigger fsmSetup
  virtual void handleStopOperation(LifecycleOperation *operation) override;
  virtual void handleCrashOperation(LifecycleOperation *operation) override;
};
}  // namespace rover
