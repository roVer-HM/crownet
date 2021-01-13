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

#include "AidSocket.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#endif  // ifdef WITH_IPv4
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "crownet/aid/Aid.h"

namespace crownet {

AidSocket::AidSocket() { socketId = getEnvir()->getUniqueNumber(); }

AidSocket::~AidSocket() {}

// ISocket Interface
bool AidSocket::belongsToSocket(cMessage *msg) const {
  auto &tags = getTags(msg);
  auto socketInd = tags.findTag<SocketInd>();
  return socketInd != nullptr && socketInd->getSocketId() == socketId;
}

void AidSocket::processMessage(cMessage *msg) {
  ASSERT(belongsToSocket(msg));

  switch (msg->getKind()) {
    case AID_I_DATA:
      if (cb)
        cb->socketDataArrived(this, check_and_cast<Packet *>(msg));
      else
        delete msg;
      break;
    case AID_I_ERROR:
      if (cb)
        cb->socketErrorArrived(this, check_and_cast<Indication *>(msg));
      else
        delete msg;
      break;
    case AID_I_STATUS:
      if (cb)
        cb->socketStatusArrived(this, check_and_cast<Indication *>(msg));
      else
        delete msg;
      break;
    case AID_I_CLOSE:
      if (cb) cb->socketClosed(this);
      socketState = CLOSED;
      delete msg;
      break;
    default:
      throw cRuntimeError(
          "AidSocket: invalid msg kind %d, one of the AID_I_xxx constants "
          "expected",
          msg->getKind());
      break;
  }
}
void AidSocket::close() {
  auto request = new Request("close", AID_C_CLOSE);
  AidCloseCommand *ctrl = new AidCloseCommand();
  request->setControlInfo(ctrl);
  sendToAid(request);
  socketState = CONNECTED;
}

void AidSocket::destroy() {
  auto request = new Request("destroy", AID_C_DESTROY);
  auto ctrl = new AidDestroyCommand();
  request->setControlInfo(ctrl);
  sendToAid(request);
}

void AidSocket::bind(int localPort) { bind(L3Address(), localPort); }

void AidSocket::bind(L3Address localAddr, int localPort) {
  if (localPort < -1 || localPort > 65535)  // -1: ephemeral port
    throw cRuntimeError("UdpSocket::bind(): invalid port number %d", localPort);

  AidBindCommand *ctrl = new AidBindCommand();
  ctrl->setLocalAddr(localAddr);
  ctrl->setLocalPort(localPort);
  auto request = new Request("bind", AID_C_BIND);
  request->setControlInfo(ctrl);
  sendToAid(request);
  socketState = INITIALIZE;
}

void AidSocket::setAppRequirements(double minRate, double maxRate,
                                   AidRecipientClass reClass, L3Address remote,
                                   int remotePort) {
  if (remote.isUnspecified())
    throw cRuntimeError("UdpSocket::connect(): unspecified remote address");
  if (remotePort <= 0 || remotePort > 65535)
    throw cRuntimeError("UdpSocket::connect(): invalid remote port number %d",
                        remotePort);

  AidAppReqCommand *ctrl = new AidAppReqCommand();
  ctrl->setMinRate(minRate);
  ctrl->setMaxRate(maxRate);
  ctrl->setRecipientClass(reClass);
  ctrl->setRemoteAddr(remote);
  ctrl->setRemotePort(remotePort);
  auto request = new Request("appRequirements", AID_C_APP_REQ);
  request->setControlInfo(ctrl);
  sendToAid(request);
  socketState = INITIALIZE;
}

void AidSocket::setAppCapabilities() {
  // todo nothing defined jet.
  AidAppCapCommand *ctrl = new AidAppCapCommand();
  auto request = new Request("appCapabilities", AID_C_APP_CAP);
  request->setControlInfo(ctrl);
  sendToAid(request);
  socketState = INITIALIZE;
}

void AidSocket::connect() {
  AidConnectCommand *ctrl = new AidConnectCommand();
  auto request = new Request("AidConnectCommand", AID_C_CONNECT);
  request->setControlInfo(ctrl);
  sendToAid(request);
  socketState = CONNECTED;
}

// AidSocket Interface
void AidSocket::sendTo(Packet *pk, L3Address destAddr, int destPort) {
  pk->setKind(AID_C_DATA);
  auto addressReq = pk->addTagIfAbsent<L3AddressReq>();
  addressReq->setDestAddress(destAddr);
  if (destPort != -1) pk->addTagIfAbsent<L4PortReq>()->setDestPort(destPort);
  sendToAid(pk);
  socketState = CONNECTED;
}

void AidSocket::send(Packet *pk) {
  pk->setKind(AID_C_DATA);
  sendToAid(pk);
  socketState = CONNECTED;
}
void AidSocket::sendToAid(cMessage *msg) {
  if (!gateToAid)
    throw cRuntimeError(
        "AidSocket: setOutputGate() must be invoked before socket can be used");

  cObject *ctrl = msg->getControlInfo();
  EV_TRACE << "AIDSocket: Send (" << msg->getClassName() << ")"
           << msg->getFullName();
  if (ctrl)
    EV_TRACE << "  control info: (" << ctrl->getClassName() << ")"
             << ctrl->getFullName();
  EV_TRACE << endl;

  auto &tags = getTags(msg);
  tags.addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Aid::aid);
  tags.addTagIfAbsent<SocketReq>()->setSocketId(socketId);
  check_and_cast<cSimpleModule *>(gateToAid->getOwnerModule())
      ->send(msg, gateToAid);
}

}  // namespace crownet
