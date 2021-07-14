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

#ifndef __INET_UDPMESSAGEHANDLER_H_
#define __INET_UDPMESSAGEHANDLER_H_

#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

// Fix re-declaration error
#undef NaN

#include "../definitions.pb.h"

namespace crownet {

using com::example::peopledensitymeasurementprototype::model::proto::SingleLocationData;
using namespace inet;

class OutboundEmulation : public ApplicationBase, public UdpSocket::ICallback
{
  public:
    virtual ~OutboundEmulation();

  protected:
    UdpSocket socketExt;
    UdpSocket socket;


    simtime_t startTime;
    simtime_t stopTime;
    cMessage *selfMsg = nullptr;

    int localPort;
    int externalPort;
    const char* localAddress;
    const char* externalAddress;
    int offsetNorthing;
    int offsetEasting;

    cOutVector *numMessages = nullptr;

    enum SelfMsgKinds { START = 1, STOP };

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;

    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

    virtual void processStart();
    virtual void processStop();

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  private:
    virtual void sendSingleLocationBroadcast(UdpSocket socket, SingleLocationData *data);
};

}

#endif
