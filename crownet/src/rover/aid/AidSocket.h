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

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/ISocket.h"
#include "rover/aid/AidCommand_m.h"

using namespace inet;

namespace rover {
class AidSocket : public ISocket {
 public:
  class ICallback {
   public:
    virtual ~ICallback() {}

    /**
     * Notifies about data arrival, packet ownership is transferred to the
     * callee.
     */
    virtual void socketDataArrived(AidSocket *socket, Packet *packet) = 0;

    /**
     * Notifies about error indication arrival, indication ownership is
     * transferred to the callee.
     */
    virtual void socketErrorArrived(AidSocket *socket,
                                    Indication *indication) = 0;

    virtual void socketStatusArrived(AidSocket *socket,
                                     Indication *indication) = 0;

    /**
     * Notifies about socket closed, indication ownership is transferred to the
     * callee.
     */
    virtual void socketClosed(AidSocket *socket) = 0;
  };
  enum State { INITIALIZE, CONNECTED, CLOSED };

 protected:
  int socketId;
  ICallback *cb = nullptr;
  State socketState = CLOSED;
  cGate *gateToAid;

 public:
  AidSocket();
  virtual ~AidSocket();

  /**
   * Set a callback object to process Messages arrived at the socket.
   * The socket does not delte the callback.
   */
  void setCallback(ICallback *cb) { this->cb = cb; }

  // ISocket Interface
  int getSocketId() const override { return socketId; }
  bool belongsToSocket(cMessage *msg) const override;
  void processMessage(cMessage *msg) override;
  void close() override;
  void destroy() override;
  bool isOpen() const override { return socketState != CLOSED; }

  void send(Packet *pk);
  void sendTo(Packet *pk, L3Address destAddr, int destPort);
  void setOutputGate(cGate *toAid) { gateToAid = toAid; }

  /**
   * Bind the socket to a local port number. Use port=0 for ephemeral port.
   */
  void bind(int localPort);

  /**
   * Bind the socket to a local port number and IP address (useful with
   * multi-homing or multicast addresses). Use port=0 for an ephemeral port.
   */
  void bind(L3Address localAddr, int localPort);

  /**
   * Register application
   */
  void setAppRequirements(double minRate, double maxRate,
                          AidRecipientClass reClass, L3Address remote,
                          int remotePort);

  void setAppCapabilities();

  /**
   * Connect to AID after requirements and capabilities are set.
   */
  void connect();

 protected:
  void sendToAid(cMessage *msg);
};
}  // namespace rover
