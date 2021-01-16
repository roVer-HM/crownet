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
#include <type_traits>
#include "inet/common/INETDefs.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/ISocket.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "crownet/aid/Aid.h"
#include "crownet/aid/AidConnection.h"

using namespace inet;

namespace crownet {

class AidConnection;
class Aid;

class SocketHandlerException : public std::exception {
 public:
  SocketHandlerException(const char *info) : info(info){};
  const char *getInfo() { return info; }

 private:
  const char *info;
};

/**
 * Wrapper for any kind of Socket to pass arrived packages through the
 * ~AidConnection message processing pipeline. The SocketHandlerException is
 * used to catch any kind of processing error which will case ~process to return
 * false.
 */
class SocketHandler : public ISocket {
 public:
  SocketHandler(ISocket *socket, AidConnection *aidConn, Aid *aidMain)
      : socket(socket), aidConn(aidConn), aidMain(aidMain){};
  virtual ~SocketHandler();
  template <class S>
  S getSocket();

 protected:
  ISocket *socket;
  AidConnection *aidConn;  // AidConnection module (do not delete);
  Aid *aidMain;            // Aid module (do not delete);

 public:
  // ISocket Interface
  virtual int getSocketId() const override;
  virtual bool belongsToSocket(cMessage *msg) const override;
  virtual void processMessage(cMessage *msg) override;
  virtual void close() override;
  virtual void destroy() override;
  virtual bool isOpen() const override;
  // SocketHandler Interface
  virtual void send(Packet *pk) = 0;
  virtual void sendTo(Packet *pk, L3Address destAddr, int destPort) = 0;

  /** process arrived socket message/packet using the aidConn.
   * SocketHandlerException are caught here. process(cMessage *msg) will return
   * false in this cases */
  virtual bool process(cMessage *msg);
  void setAidConnection(AidConnection *conn) { aidConn = conn; }
  void setAidModule(Aid *module) { aidMain = module; }
};

class UdpSocketHandler : public SocketHandler, public UdpSocket::ICallback {
 public:
  UdpSocketHandler(ISocket *socket, AidConnection *aidConn, Aid *aidMain);
  virtual ~UdpSocketHandler(){};
  static UdpSocketHandler *create(L3Address localAddr, int localPort);

 public:
  virtual void send(Packet *pk) override;
  virtual void sendTo(Packet *pk, L3Address destAddr, int destPort) override;

  // UdpSocket::ICallback
  virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
  virtual void socketErrorArrived(UdpSocket *socket,
                                  Indication *indication) override;
  virtual void socketClosed(UdpSocket *socket) override;
};

}  // namespace crownet
