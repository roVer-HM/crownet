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

#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "crownet/applications/common/BaseSocketManager.h"
using namespace inet;

namespace crownet {

class UdpSocketManager : public BaseSocketManager, public UdpSocket::ICallback{
public:
    UdpSocketManager();
    virtual ~UdpSocketManager();

protected:
 UdpSocket socket;

protected:
 virtual void initSocket() override;
 virtual ISocket &getSocket() override;

 // UdpSocket::ICallback
 virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
 virtual void socketErrorArrived(UdpSocket *socket,
                                 Indication *indication) override;
 virtual void socketClosed(UdpSocket *socket) override;

};
}

