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

#include "crownet/applications/common/BaseSocketManager.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/ModuleAccess.h"

namespace crownet {



BaseSocketManager::BaseSocketManager() {}

BaseSocketManager::~BaseSocketManager() {}

void BaseSocketManager::initialize(int stage) {
    OperationalBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL){
        localPort = par("localPort").intValue();
        destPort = par("destPort").intValue();
        dontFragment = par("dontFragment").boolValue();
    }
}

void BaseSocketManager::setupSocket(){
    Enter_Method_Silent();
    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;

    while ((token = tokenizer.nextToken()) != nullptr) {
      destAddressStr.push_back(token);
      L3Address result;
      L3AddressResolver().tryResolve(token, result);
      if (result.isUnspecified())
        EV_ERROR << "cannot resolve destination address: " << token << endl;
      destAddresses.push_back(result);
    }

    initSocket();


}

void BaseSocketManager::pushPacket(Packet *packet, cGate *gate) {
    // called by application (using some shaper)
    take(packet);
    sendTo(packet);
}

void BaseSocketManager::sendTo(Packet *pk){
    if (dontFragment) pk->addTag<FragmentationReq>()->setDontFragment(true);
    if(!pk->findTag<L3AddressReq>()){
        auto addrReq = pk->addTagIfAbsent<L3AddressReq>();
        addrReq->setDestAddress(chooseDestAddr());
    }
    if(!pk->findTag<L4PortReq>()){
        auto portReq = pk->addTagIfAbsent<L4PortReq>();
        portReq->setDestPort(destPort);
    }
    getSocket().send(pk);
}

void BaseSocketManager::handleMessageWhenUp(cMessage *msg){

    if (msg->arrivedOn("fromStack")){
       getSocket().processMessage(msg);
    }
     else if (msg->arrivedOn("in")) {
       // app may already set L3Address and port. If missing SocketManager will add them.
       sendTo(check_and_cast<Packet *>(msg));
    } else if (msg->isSelfMessage()){
        throw omnetpp::cRuntimeError("SocketManager does not have self messages");
    }
}

L3Address BaseSocketManager::chooseDestAddr() {
  int k = intrand(destAddresses.size());
  if (destAddresses[k].isUnspecified() || destAddresses[k].isLinkLocal()) {
    L3AddressResolver().tryResolve(destAddressStr[k].c_str(), destAddresses[k]);
  }
  return destAddresses[k];
}

// Lifecycle management

void BaseSocketManager::handleStartOperation(LifecycleOperation *operation) {
   // todo: check nothing to do
}

void BaseSocketManager::handleStopOperation(LifecycleOperation *operation) {
    // todo: check nothing to do
}

void BaseSocketManager::handleCrashOperation(LifecycleOperation *operation) {
    // todo: check nothing to do
}

//socket api
int BaseSocketManager::getSocketId() {
    Enter_Method_Silent();
    return getSocket().getSocketId();
}
bool BaseSocketManager::belongsToSocket(cMessage *msg) {
    Enter_Method_Silent();
    return getSocket().belongsToSocket(msg);
}
void BaseSocketManager::close() {
    Enter_Method_Silent();
    getSocket().close();
}
void BaseSocketManager::destroy() {
    Enter_Method_Silent();
    getSocket().destroy();
}
bool BaseSocketManager::isOpen() {
    Enter_Method_Silent();
    return getSocket().isOpen();
}


}

