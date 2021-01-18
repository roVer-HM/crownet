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

#include "Aid.h"

#include "inet/common/socket/SocketTag_m.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/InitStages.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Message.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "crownet/aid/AidConnection.h"
#include "crownet/common/SocketHandler.h"


using namespace inet;

namespace crownet {

Define_Module(Aid);

Aid::Aid() {}

Aid::~Aid() {}

/** Static **/

// Custom protocol identifiers
const Protocol Aid::aid("aid", "AID");
// use INITSTAGE_ROUTING_PROTOCOLS to be after transport and before
// application.
const InitStages Aid::INITSTAGE_AID_LAYER = INITSTAGE_ROUTING_PROTOCOLS;
// use STAGE_ROUTING_PROTOCOLS to be after transport and before
// application.
const ModuleStartOperation::Stage Aid::STAGE_AID_PROTOCOLS =
    ModuleStartOperation::STAGE_ROUTING_PROTOCOLS;

/** LayeredProtocolBase **/

void Aid::initialize(int stage) {
  LayeredProtocolBase::initialize(stage);
  if (stage == INITSTAGE_LOCAL) {
  } else if (stage == INITSTAGE_AID_LAYER) {
    // after Transport before Applications.

    // register AID service and protocol for IProtocolRegistrationListener
    // (i.e. Dispatcher)
    registerServiceAndProtocol();
  }
}

void Aid::registerServiceAndProtocol() {
  registerService(Aid::aid, gate("upperIn"), gate("lowerIn"));
  registerProtocol(Aid::aid, gate("lowerOut"), gate("upperOut"));
}

bool Aid::isUpperMessage(cMessage *msg) { return msg->arrivedOn("upperIn"); }

bool Aid::isLowerMessage(cMessage *msg) { return msg->arrivedOn("lowerIn"); }

bool Aid::isInitializeStage(int stage) { return stage == INITSTAGE_AID_LAYER; }

bool Aid::isModuleStartStage(int stage) { return stage == STAGE_AID_PROTOCOLS; }

bool Aid::isModuleStopStage(int stage) { return stage == STAGE_AID_PROTOCOLS; }

/** LayeredProtocolBase End **/

/** ILifeCycle **/

void Aid::handleStartOperation(LifecycleOperation *operation) {
  // todo startup operation
}

void Aid::handleStopOperation(LifecycleOperation *operation) {
  // todo stop operation
  reset();
  delayActiveOperationFinish(par("stopOperationTimeout"));
  startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void Aid::handleCrashOperation(LifecycleOperation *operation) { reset(); }

// called at shutdown/crash
void Aid::reset() {
  for (auto &elem : aidAppConnMap) elem.second->deleteModule();
  aidAppConnMap.clear();
}

/** ILifeCycle End**/

/** Message handling **/

void Aid::handleSelfMessage(cMessage *msg) {
  throw cRuntimeError("model error: should schedule timers on connection");
}

void Aid::handleUpperCommand(cMessage *msg) {
  Message *message = check_and_cast<Message *>(msg);
  int socketId = message->getTags().getTag<SocketReq>()->getSocketId();
  AidConnection *conn = findConnForApp(socketId);

  if (!conn) {
    conn = createConnection(socketId);

    // add into appConnMap here; it'll be added to connMap during processing
    aidAppConnMap[socketId] = conn;

    EV_INFO << "Aid connection created for " << msg << "\n";
  }

  if (!conn->processAppCommand(msg)) removeConnection(conn);
}

void Aid::handleUpperPacket(Packet *packet) { handleUpperCommand(packet); }

void Aid::handleLowerCommand(cMessage *message) {
  ISocket *socket = transportSockets.findSocketFor(message);
  AidConnection *conn = findConnForApp(socket->getSocketId());
  SocketHandler *socketHandler = dynamic_cast<SocketHandler *>(socket);
  if (!socketHandler) cRuntimeError("transportSockets must be SocketHandler");

  bool ret = socketHandler->process(message);
  if (!ret) removeConnection(conn);
}

void Aid::handleLowerPacket(Packet *packet) { handleLowerCommand(packet); }

/** Message handling End**/

/** Utils and factories **/

AidConnection *Aid::createConnection(int appSocketId) {
  auto moduleType = cModuleType::get("crownet.aid.AidConnection");
  char submoduleName[24];
  sprintf(submoduleName, "conn-%d", appSocketId);
  auto module = check_and_cast<AidConnection *>(
      moduleType->createScheduleInit(submoduleName, this));
  module->initConnection(this, appSocketId);
  return module;
}

SocketHandler *Aid::createSocketHandler(AidConnection *conn,
                                        const SocketType &type) {
  Enter_Method_Silent();
  SocketHandler *handler = nullptr;
  switch (type) {
    case UDP: {
      UdpSocket *s = new UdpSocket();
      s->setOutputGate(gate("lowerOut"));
      s->bind(conn->localAddr, conn->localPort);

      // socketOptions
      int timeToLive = par("timeToLive");
      if (timeToLive != -1) s->setTimeToLive(timeToLive);

      int dscp = par("dscp");
      if (dscp != -1) s->setDscp(dscp);

      int tos = par("tos");
      if (tos != -1) s->setTos(tos);

      const char *multicastInterface = par("multicastInterface");
      if (multicastInterface[0]) {
        IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(
            par("interfaceTableModule"), this);
        NetworkInterface *ie = ift->findInterfaceByName(multicastInterface);
        if (!ie)
          throw cRuntimeError(
              "Wrong multicastInterface setting: no interface named \"%s\"",
              multicastInterface);
        s->setMulticastOutputInterface(ie->getInterfaceId());

        bool receiveBroadcast = par("receiveBroadcast");
        if (receiveBroadcast) s->setBroadcast(true);

        bool joinLocalMulticastGroups = par("joinLocalMulticastGroups");
        if (joinLocalMulticastGroups) {
          MulticastGroupList mgl = getModuleFromPar<IInterfaceTable>(
                                       par("interfaceTableModule"), this)
                                       ->collectMulticastGroups();
          s->joinLocalMulticastGroups(mgl);
        }

        handler = new UdpSocketHandler(s, conn, this);
        break;
      }
    } break;
    case SocketType::TCP: {
      throw cRuntimeError("notImplementent");
      break;
    }
  }

  if (!handler) throw cRuntimeError("Error setup SocketHandler");
  transportSockets.addSocket(handler);
  return handler;
}

AidConnection *Aid::findConnForApp(int appSocketId) {
  auto i = aidAppConnMap.find(appSocketId);
  return i == aidAppConnMap.end() ? nullptr : i->second;
}

void Aid::removeConnection(AidConnection *conn) {
  Enter_Method_Silent();
  EV_INFO << "Deleting Aid Connection" << endl;
  aidAppConnMap.erase(conn->getAppSocketId());
}

void Aid::sendFromConn(cMessage *msg, const char *gatename, int gateindex) {
  Enter_Method_Silent();
  take(msg);
  send(msg, gatename, gateindex);
}
/** Utils and factories **/

/** Utils GUI **/
void Aid::refreshDisplay() const {
  OperationalBase::refreshDisplay();

  char buf[80];
  sprintf(buf, "passed up: %d pks\nsent: %d pks", numPassedUp, numSent);
  if (numDroppedWrongPort > 0) {
    sprintf(buf + strlen(buf), "\ndropped (no app): %d pks",
            numDroppedWrongPort);
    getDisplayString().setTagArg("i", 1, "red");
  }
  getDisplayString().setTagArg("t", 0, buf);
}
/** Utils GUI End**/
}  // namespace crownet
