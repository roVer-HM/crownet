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

#include "SocketHandler.h"
#include "rover/aid/Aid.h"

using namespace inet;

namespace rover {

SocketHandler::~SocketHandler() { delete socket; }

template <>
UdpSocket *SocketHandler::getSocket<UdpSocket *>() {
  return static_cast<UdpSocket *>(socket);
}

int SocketHandler::getSocketId() const { return socket->getSocketId(); }

bool SocketHandler::belongsToSocket(cMessage *msg) const {
  return socket->belongsToSocket(msg);
}

void SocketHandler::processMessage(cMessage *msg) {
  return socket->processMessage(msg);
}

void SocketHandler::close() { socket->close(); }

void SocketHandler::destroy() { socket->destroy(); }

bool SocketHandler::isOpen() const { return socket->isOpen(); }

bool SocketHandler::process(cMessage *msg) {
  try {
    processMessage(msg);
  } catch (SocketHandlerException &ex) {
    EV_ERROR << ex.getInfo() << std::endl;
    return false;
  }
  return true;
}
/** UdpSocketHanlder **/

UdpSocketHandler::UdpSocketHandler(ISocket *socket, AidConnection *aidConn,
                                   Aid *aidMain)
    : SocketHandler(socket, aidConn, aidMain) {
  getSocket<UdpSocket *>()->setCallback(this);
};

void UdpSocketHandler::send(Packet *pk) {
  UdpSocket *s = getSocket<UdpSocket *>();
  s->send(pk);
}

void UdpSocketHandler::sendTo(Packet *pk, L3Address destAddr, int destPort) {
  UdpSocket *s = getSocket<UdpSocket *>();
  s->sendTo(pk, destAddr, destPort);
}

void UdpSocketHandler::socketDataArrived(UdpSocket *s, Packet *packet) {
  bool ret = aidConn->processLower(packet, s->getSocketId());
  if (!ret)
    throw SocketHandlerException("aidConn->processLower returned false");
}

void UdpSocketHandler::socketErrorArrived(UdpSocket *socket,
                                          Indication *indication) {
  PacketDropDetails details;
  details.setReason(INCORRECTLY_RECEIVED);
  aidMain->emit(packetDroppedSignal, indication, &details);
  delete indication;
}
void UdpSocketHandler::socketClosed(UdpSocket *socket) {}

}  // namespace rover
