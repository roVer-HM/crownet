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
#include "../../mobility/IPositionHistoryProvider.h"
#include "../../mobility/IPositionEmulator.h"

namespace crownet {

using namespace inet;

class NodeLocationExporter : public ApplicationBase
{
  public:
    virtual ~NodeLocationExporter();

  protected:
    UdpSocket socketExt;

    simtime_t startTime;
    simtime_t stopTime;

    cMessage *selfMsg = nullptr;

    int port;
    const char* address;
    int xOffset;
    int yOffset;
    double interval;

    IPositionHistoryProvider* mobilityModule = nullptr;

    enum SelfMsgKinds { START = 1, STOP, UPDATE };

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;

    virtual void processStart();
    virtual void processStop();

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  private:
    void sendLocationPacket();

    int lastX = 0;
    int lastY = 0;
};

}

#endif
