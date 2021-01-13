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

#include "crownet/aid/AidSocket.h"
#include "crownet/applications/common/BaseApp.h"

using namespace inet;

namespace crownet {
class AidBaseApp : public BaseApp, public AidSocket::ICallback {
 public:
  AidBaseApp();
  virtual ~AidBaseApp();

 protected:
  AidSocket socket;

 protected:
  virtual void initialize(int stage) override;

  virtual FsmState fsmHandleSelfMsg(cMessage *msg) override;

  // socket actions.
  virtual void initSocket() override;
  virtual ISocket &getSocket() override;
  virtual void sendToSocket(Packet *msg, L3Address destAddr,
                            int destPort) override;
  virtual void setAppRequirements() = 0;
  virtual void setAppCapabilities() = 0;

  // AidSocket::ICallback
  virtual void socketDataArrived(AidSocket *socket, Packet *packet) override;
  virtual void socketErrorArrived(AidSocket *socket,
                                  Indication *indication) override;
  virtual void socketClosed(AidSocket *socket) override;
  virtual void socketStatusArrived(AidSocket *socket,
                                   Indication *indication) override;
};
}  // namespace crownet
