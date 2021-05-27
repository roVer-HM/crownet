
#pragma once

#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/ISocket.h"


using namespace inet;

namespace crownet {

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
using FsmState = int;  // state for each FSM.

class DataArrivedHandler {
public:
    virtual ~DataArrivedHandler() = default;
    virtual FsmState handleDataArrived(Packet *packet) = 0;
};

class SocketHandler {
public:
    virtual ~SocketHandler() = default;
    virtual void setupSocket();
    virtual ISocket &getSocket() = 0;
    virtual void sendTo(Packet *pk) =0;
    virtual int getLocalPort() = 0;
    virtual int getDestPort() = 0;
    virtual bool hasDestAddress() const;
};

}
