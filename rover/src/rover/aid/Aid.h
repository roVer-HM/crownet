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

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/Protocol.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/socket/SocketMap.h"

using namespace inet;

namespace rover {

class AidConnection;
class SocketHandler;

enum SocketType { UDP, TCP };

class Aid : public LayeredProtocolBase {
  friend AidConnection;
  friend SocketHandler;

 public:
  // custom static const inet::Protocol for AID
  static const Protocol aid;  // adaptive information dissemination

  // init stage for AID
  static const InitStages INITSTAGE_AID_LAYER;
  // start operation stage for AID
  static const ModuleStartOperation::Stage STAGE_AID_PROTOCOLS;

 protected:
  /** LayeredProtocolBase **/
  virtual int numInitStages() const override { return NUM_INIT_STAGES; }
  virtual void initialize(int stage) override;
  virtual void registerServiceAndProtocol();

  virtual bool isUpperMessage(cMessage *message) override;
  virtual bool isLowerMessage(cMessage *message) override;

  virtual bool isInitializeStage(int stage) override;
  virtual bool isModuleStartStage(int stage) override;
  virtual bool isModuleStopStage(int stage) override;
  /** LayeredProtocolBase End**/

  /** ILifeCycle **/
  virtual void handleStartOperation(LifecycleOperation *operation) override;
  virtual void handleStopOperation(LifecycleOperation *operation) override;
  virtual void handleCrashOperation(LifecycleOperation *operation) override;
  // called at shutdown/crash
  virtual void reset();
  /** ILifeCycle End**/

  /** Message handling **/
  virtual void handleSelfMessage(cMessage *message) override;
  virtual void handleUpperCommand(cMessage *message) override;
  virtual void handleUpperPacket(Packet *packet) override;
  virtual void handleLowerCommand(cMessage *message) override;
  virtual void handleLowerPacket(Packet *packet) override;
  /** Message handling End**/

  /** Utils and factories **/
  virtual AidConnection *createConnection(int appSocketId);
  virtual SocketHandler *createSocketHandler(AidConnection *conn,
                                             const SocketType &type);
  virtual AidConnection *findConnForApp(int appSocketId);
  virtual void removeConnection(AidConnection *conn);
  virtual void sendFromConn(cMessage *msg, const char *gatename,
                            int gateindex = -1);
  /** Utils and factories **/

  /** Utils GUI **/
  virtual void refreshDisplay() const override;
  /** Utils GUI End**/

 public:
  Aid();
  virtual ~Aid();

 protected:
  /** state **/
  typedef std::map<int /*(App)socketId*/, AidConnection *> AidAppConnMap;

  AidAppConnMap aidAppConnMap;
  SocketMap transportSockets;

  /** statistics **/
  int numSent = 0;
  int numPassedUp = 0;
  int numDroppedWrongPort = 0;
  int numDroppedBadChecksum = 0;
};
}  // namespace rover
