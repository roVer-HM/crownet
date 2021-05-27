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

#include "AidSocketManager.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "crownet/applications/common/AppCommon_m.h"
#include "crownet/applications/common/AppFsm.h"

namespace crownet {

Define_Module(AidSocketManager);

AidSocketManager::AidSocketManager() {
    // TODO Auto-generated constructor stub

}

AidSocketManager::~AidSocketManager() {
    // TODO Auto-generated destructor stub
}

void AidSocketManager::initSocket() {
  socket.setOutputGate(gate("toStack"));
  const char *localAddress = par("localAddress");
  socket.bind(
      *localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(),
      localPort);

  setAppRequirements();
  setAppCapabilities();
  socket.connect();

  socket.setCallback(this);
}

void AidSocketManager::setAppRequirements(){
    L3Address destAddr = chooseDestAddr();
    socket.setAppRequirements(par("minRate").doubleValue(),
                              par("maxRate").doubleValue(),
                              AidRecipientClass::ALL_LOCAL, destAddr, destPort);
}

void AidSocketManager::setAppCapabilities(){
    // do nothing
    // todo: no CAP right now.
}

ISocket &AidSocketManager::getSocket() { return socket; }


// AidSocket::ICallback
void AidSocketManager::socketDataArrived(AidSocket *socket, Packet *packet) {
  emit(packetReceivedSignal, packet);
  send(packet, gate("toApp"));
}

void AidSocketManager::socketErrorArrived(AidSocket *socket, Indication *indication) {
  EV_WARN << "Ignoring Aid error report " << indication->getName() << endl;
  // todo handle socketError
  delete indication;
}

void AidSocketManager::socketStatusArrived(AidSocket *socket,
                                     Indication *indication) {
  EV_WARN << "Ignoring AID Status" << endl;
  // todo handle indication (i.e. reduce frequency of app)
  delete indication;
}

void AidSocketManager::socketClosed(AidSocket *socket) {
  if (operationalState == State::STOPPING_OPERATION) {
    startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
  }
  // todo inform app logiv of socketClosed
}



}
