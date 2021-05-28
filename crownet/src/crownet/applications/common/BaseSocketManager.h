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

#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"

#include "crownet/common/result/Simsignals.h"
#include "inet/common/INETDefs.h"
#include "inet/common/socket/ISocket.h"
#include "inet/networklayer/common/L3Address.h"

#include "crownet/applications/common/AppFsm.h"

using namespace inet;


namespace crownet {


#ifndef CROWNET_APPLICATIONS_COMMON_BASESOCKETMANAGER_H_
#define CROWNET_APPLICATIONS_COMMON_BASESOCKETMANAGER_H_

class BaseSocketManager: public OperationalBase, public SocketProvider {
public:
  BaseSocketManager();
  virtual ~BaseSocketManager();
  virtual void initialize(int stage) override;

  virtual void sendTo(Packet *pk);

  virtual void setupSocket() override;
  virtual int getLocalPort() override {return localPort;}
  virtual int getDestPort() override {return destPort;}
  virtual bool hasDestAddress() override {return destAddresses.empty();}

  //socket api
  virtual int getSocketId() override;
  virtual bool belongsToSocket(cMessage *msg) override;
  virtual void close() override;
  virtual void destroy() override;
  virtual bool isOpen() override;

protected:
  virtual bool isInitializeStage(int stage) override { return stage == INITSTAGE_TRANSPORT_LAYER; }
  virtual bool isModuleStartStage(int stage) override { return stage == ModuleStartOperation::STAGE_TRANSPORT_LAYER; }
  virtual bool isModuleStopStage(int stage) override { return stage == ModuleStopOperation::STAGE_TRANSPORT_LAYER; }
  virtual void handleMessageWhenUp(cMessage *msg) override;

  L3Address chooseDestAddr();

  virtual void initSocket() = 0;
  virtual ISocket& getSocket() = 0;

  // Lifecycle management
  virtual void handleStartOperation(
      LifecycleOperation *operation) override;  // trigger fsmSetup
  virtual void handleStopOperation(LifecycleOperation *operation) override;
  virtual void handleCrashOperation(LifecycleOperation *operation) override;

protected:
  int localPort = -1;
  int destPort = -1;
  std::vector<L3Address> destAddresses;
  std::vector<std::string> destAddressStr;
};
}

#endif /* CROWNET_APPLICATIONS_COMMON_BASESOCKETMANAGER_H_ */
