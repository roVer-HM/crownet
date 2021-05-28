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

#ifndef CROWNET_APPLICATIONS_COMMON_AIDSOCKETMANAGER_H_
#define CROWNET_APPLICATIONS_COMMON_AIDSOCKETMANAGER_H_

#include "crownet/applications/common/BaseSocketManager.h"
#include "crownet/aid/AidSocket.h"
using namespace inet;


namespace crownet {


class AidSocketManager: public BaseSocketManager, public AidSocket::ICallback{
public:
    AidSocketManager();
    virtual ~AidSocketManager();

protected:
 AidSocket socket;

public:
 virtual void initSocket() override;


protected:
 virtual void setAppRequirements();
 virtual void setAppCapabilities();

 // AidSocket::ICallback
 virtual void socketDataArrived(AidSocket *socket, Packet *packet) override;
 virtual void socketErrorArrived(AidSocket *socket,
                                 Indication *indication) override;
 virtual void socketClosed(AidSocket *socket) override;
 virtual void socketStatusArrived(AidSocket *socket,
                                  Indication *indication) override;

 // Socket
 virtual ISocket &getSocket() override;

};
}

#endif /* CROWNET_APPLICATIONS_COMMON_AIDSOCKETMANAGER_H_ */
