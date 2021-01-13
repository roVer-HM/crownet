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

#include "AidBaseApp.h"

using namespace inet;

namespace rover {
AidBaseApp::AidBaseApp() {
  // TODO Auto-generated constructor stub
}

AidBaseApp::~AidBaseApp() {
  // TODO Auto-generated destructor stub
}

void AidBaseApp::initialize(int stage) { BaseApp::initialize(stage); }

BaseApp::FsmState AidBaseApp::fsmHandleSelfMsg(cMessage *msg) {
  throw cRuntimeError("Module does not handle SelfMessages. Received Kind: %d",
                      msg->getKind());
}

void AidBaseApp::initSocket() {
  socket.setOutputGate(gate("socketOut"));
  const char *localAddress = par("localAddress");
  socket.bind(
      *localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(),
      localPort);

  setAppRequirements();
  setAppCapabilities();
  socket.connect();

  socket.setCallback(this);
}

ISocket &AidBaseApp::getSocket() { return socket; }

void AidBaseApp::sendToSocket(Packet *msg, L3Address destAddr, int destPort) {
  socket.sendTo(msg, destAddr, destPort);
}

// UdpSocket::ICallback
void AidBaseApp::socketDataArrived(AidSocket *socket, Packet *packet) {
  emit(packetReceivedSignal, packet);
  numReceived++;
  delete packet;
  setFsmResult(FsmRootStates::WAIT_ACTIVE);
}

void AidBaseApp::socketErrorArrived(AidSocket *socket, Indication *indication) {
  EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
  delete indication;
  setFsmResult(FsmRootStates::WAIT_ACTIVE);  // ignore Error
}

void AidBaseApp::socketClosed(AidSocket *socket) {
  if (operationalState == State::STOPPING_OPERATION) {
    startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
  }
  setFsmResult(FsmRootStates::TEARDOWN);
}

void AidBaseApp::socketStatusArrived(AidSocket *socket,
                                     Indication *indication) {
  EV_WARN << "Ignoring AID Status" << endl;
  delete indication;
  setFsmResult(socketFsmResult =
                   FsmRootStates::WAIT_ACTIVE);  // just go on for now
}

}  // namespace rover
