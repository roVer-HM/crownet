/*
 * InboundEmulation.h
 *
 *  Created on: Jun 23, 2021
 *      Author: mru
 */

#pragma once

#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "../../mobility/IPositionEmulator.h"

// Fix re-declaration error
#undef NaN

#include "../definitions.pb.h"

namespace crownet {

using com::example::peopledensitymeasurementprototype::model::proto::SingleLocationData;
using namespace inet;

class InboundEmulation : public ApplicationBase, public UdpSocket::ICallback
{
  public:
    virtual ~InboundEmulation();

  protected:
    UdpSocket socketExt;

    simtime_t startTime;
    simtime_t stopTime;
    cMessage *selfMsg = nullptr;

    int externalPort;
    const char* externalAddress;
    int offsetNorthing;
    int offsetEasting;

    int deviceId;

    IPositionEmulator* positionEmulator;

    enum SelfMsgKinds { START = 1, STOP, UPDATE };

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
};

}
