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

#include "inet/common/INETDefs.h"
#include "inet/common/socket/ISocket.h"
#include "inet/networklayer/common/L3Address.h"
#include "rover/aid/Aid.h"
#include "rover/aid/AidCommand_m.h"

using namespace inet;

namespace rover {

class SocketHandler;

enum AidState {
  AID_S_INIT = 0,
  AID_S_CLOSED = FSM_Steady(1),
  AID_S_BIND = FSM_Steady(2),
  AID_S_ESTABLISHED = FSM_Steady(3),
  AID_S_DESTROYED = FSM_Steady(4)

};

enum AidEventCode {
  AID_E_IGNORE,

  AID_E_BIND,
  AID_E_APP_REQ,
  AID_E_APP_CAP,
  AID_E_CONNECT,
  AID_E_DATA,
  AID_E_CLOSE,
  AID_E_DESTROY,

};

struct AppRequirments {
  double minRate;
  double maxRate;
  AidRecipientClass recipientClass;
};

class AidConnection : public cSimpleModule {
  friend Aid;

 public:
  AidConnection();
  virtual ~AidConnection();

  void initConnection(Aid* aidMain, int appSocketId);
  int getAppSocketId() const;
  int getTransSocketId() const;
  int getAidConnectionId() const;
  const L3Address& getLocalAddr() const;
  const L3Address& getRemoteAddr() const;

 private:
  int aidConnectionId = -1;  // todo: do i need this?

 protected:
  // Aid module (do not delete)
  Aid* aidMain = nullptr;
  // Aid state machine
  cFSM fsm;
  // Upper (App) connection identifier
  int appSocketId = -1;
  // Lower (Transport) connection identifier
  // (do not delete. Managed by aidMain)
  // todo: allow multiple SocketHandler for UDP and TCP
  SocketHandler* socketHandler = nullptr;

  // AID REQ/CAP/Addresses
  L3Address localAddr;
  L3Address remoteAddr;
  int localPort = -1;
  int remotePort = -1;
  // AppRequirments
  AppRequirments appRequirments;

 public:
  /** Message processing */
  // Process messages of AidConnection simple module. (only selfMessages)
  virtual void handleMessage(cMessage* msg) override;
  // Messages received from Upper (App) Layer
  virtual bool processAppCommand(cMessage* msg);
  // Messages received from Lower (Transport). Called from SocketHandler
  virtual bool processLower(Packet* packet, int receivingSocketId);
  // Internal (self)messages of the AidConnection
  virtual bool processTimer(cMessage* msg);
  /** Message processing End*/

 protected:
  /** State Machine Handling */
  // Maps app command codes (MsgKind)  to AID_E_xxx event codes
  virtual AidEventCode preanalyseAppCommandEvent(int commandCode);
  /** Implemements the pure AID state machine */
  virtual bool performStateTransition(const AidEventCode& event);
  /** Perform cleanup necessary when entering a new state, e.g. cancelling
   * timers */
  virtual void stateEntered(int state, int oldState, AidEventCode event);
  /** State Machine Handling End*/

  /** Process App Command (Sate Machine) */
  virtual void process_AID_BIND(AidEventCode& event, AidCommand* aidCommand,
                                cMessage* msg);
  virtual void process_AID_APP_REQ(AidEventCode& event, AidCommand* aidCommand,
                                   cMessage* msg);
  virtual void process_AID_APP_CAP(AidEventCode& event, AidCommand* aidCommand,
                                   cMessage* msg);
  virtual void process_AID_CONNECT(AidEventCode& event, AidCommand* aidCommand,
                                   cMessage* msg);
  virtual void process_AID_DATA(AidEventCode& event, AidCommand* aidCommand,
                                cMessage* msg);
  virtual void process_AID_CLOSE(AidEventCode& event, AidCommand* aidCommand,
                                 cMessage* msg);
  virtual void process_AID_DESTROY(AidEventCode& event, AidCommand* aidCommand,
                                   cMessage* msg);
  /** Process App Command (Sate Machine) End*/

  /** Utility */
  /** sends packet to application */
  virtual void sendToApp(cMessage* msg);
  /** printConnBrief */
  virtual void printConnBrief() const;
  /** returns name of TCP_S_xxx constants */
  static const char* stateName(int state);
  /** returns name of TCP_E_xxx constants */
  static const char* eventName(int event);
  /** returns name of TCP_I_xxx constants */
  static const char* indicationName(int code);
  /** Utility End*/
};
}  // namespace rover
