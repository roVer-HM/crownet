/*
 * UdpDetourApp.h
 *
 *  Created on: Feb 12, 2020
 *      Author: sts
 */

#pragma once
#include <vector>

#define FSM_DEBUG  // enables debug output from FSMs
#include <omnetpp.h>
#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/INETDefs.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/mobility/base/MobilityBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "rover/applications/udpapp/DetourAppPacket_m.h"
using namespace inet;

namespace rover {

struct PropagationHandle {
  IntrusivePtr<const DetourAppPacket> pkt;
  cMessage *msg;
  simtime_t restTime;
};

class UdpDetourApp : public ApplicationBase, public UdpSocket::ICallback {
 public:
  UdpDetourApp() {}
  virtual ~UdpDetourApp();

 protected:
  virtual void initialize(int stage) override;
  virtual void handleMessageWhenUp(cMessage *msg) override;
  virtual void finish() override;
  virtual void refreshDisplay() const override;

  virtual void setSocketOptions();

  virtual void handleStartOperation(LifecycleOperation *operation) override;
  virtual void handleStopOperation(LifecycleOperation *operation) override;
  virtual void handleCrashOperation(LifecycleOperation *operation) override;

  virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
  virtual void socketErrorArrived(UdpSocket *socket,
                                  Indication *indication) override;
  virtual void socketClosed(UdpSocket *socket) override;

  //
  virtual const bool isSender();
  virtual Coord getCurrentLocation();
  virtual void actOnIncident(IntrusivePtr<const DetourAppPacket> pkt);
  virtual void sendPayload(IntrusivePtr<const DetourAppPacket> payload);

  //  Finite State Machine setup omnetpp::cFSM fsm;
  omnetpp::cFSM fsm;
  enum FsmStates {
    INIT = 0,
    WAIT_ACTIVE = FSM_Steady(1),
    WAIT_INACTIVE = FSM_Steady(2),
    ERR = FSM_Steady(3),
    SETUP = FSM_Transient(101),
    TEARDOWN = FSM_Transient(102),
    INCIDENT_TX = FSM_Transient(103),
    INCIDENT_RX = FSM_Transient(104),
    PROPAGATE_TX = FSM_Transient(105),
    PROPAGATE_RX = FSM_Transient(106),
    DESTROY = FSM_Transient(107),
  };
  FsmStates nextState = FsmStates::ERR;

  FsmStates fsmSetupExit(cMessage *msg);
  FsmStates fsmTeardownExit(cMessage *msg);
  FsmStates fsmIncidentTxExit(cMessage *msg);
  FsmStates fsmIncidentRxExit(IntrusivePtr<const DetourAppPacket> pkt);
  FsmStates fsmPropagateTxExit(cMessage *msg);
  FsmStates fsmPropagateRxExit(IntrusivePtr<const DetourAppPacket> pkt);
  FsmStates fsmDestroyExit(cMessage *msg);

  // state
  UdpSocket socket;
  cMessage *selfMsg = nullptr;
  cMessage *selfMsgIncident = nullptr;     // owned
  MobilityBase *mobilityModule = nullptr;  // not managed here do not delete.
  std::map<std::string, PropagationHandle> propagationQueue;  // key=msgName

  // statistics
  int numSent = 0;
  int numReceived = 0;

  // parameters
  simtime_t incidentTime;
  std::string reason;
  int closedTarget;
  std::vector<int> alternativeRoute;
  std::vector<L3Address> destAddresses;
  std::vector<std::string> destAddressStr;
  int localPort = -1, destPort = -1;
  simtime_t startTime;
  simtime_t stopTime;
  bool dontFragment = false;
  const char *packetName = nullptr;
  simtime_t repeatTime;
};  // namespace rover

} /* namespace rover */
