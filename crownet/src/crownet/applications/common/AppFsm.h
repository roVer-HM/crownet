
#pragma once

#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/ISocket.h"


#include "crownet/applications/common/BurstInfo.h"


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
  DATA_IN = FSM_Transient(103),
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



class AppStatusInfo {
public:
    virtual ~AppStatusInfo() = default;
    virtual const bool isRunning() = 0;
    virtual const bool isStopped() = 0;
    virtual const FsmState getState() = 0;
    // ensure packet can be created due to enough scheduled data or enough
    // 'new' data ready for transition
    virtual const bool canProducePacket() = 0;
    // set budget the application can use. -1b default to all available data.
    virtual void setScheduleData(inet::b) = 0;
    // get available data the application can use to create packets.
    virtual const inet::b getScheduleData() = 0;
    // return available length for the next packet taking into account max MTU and
    // provided data budget.
    virtual BurstInfo getBurstInfo(inet::b) const { return BurstInfo{1, inet::b(-1)};}
    virtual const inet::b getAvailablePduLenght() = 0;
    virtual const inet::b getMinPdu() const = 0;
    virtual const inet::b getMaxPdu() const = 0;
    virtual const simtime_t getStartTime() = 0;
    virtual const simtime_t getStopTime() = 0;
};

class SocketProvider{
public:
    virtual ~SocketProvider() = default;
    virtual void setupSocket() = 0;
    virtual int getLocalPort() = 0;
    virtual int getDestPort() = 0;
    virtual bool hasDestAddress() = 0;
    virtual int getSocketId() = 0;
    virtual bool belongsToSocket(cMessage *msg) = 0;
    virtual void close() = 0;
    virtual void destroy() = 0;
    virtual bool isOpen() = 0;
};

}
