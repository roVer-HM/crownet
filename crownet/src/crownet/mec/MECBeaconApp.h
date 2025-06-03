#ifndef __MECBEACONAPP_H_
#define __MECBEACONAPP_H_

#include "omnetpp.h"

#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"

#include "packets/SimpleMECBeacon_m.h"
#include <apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_m.h>
#include <apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_Types.h>


#include <nodes/mec/MECPlatform/ServiceRegistry/ServiceRegistry.h>

#include <apps/mec/MecApps/MecAppBase.h>
#include "aggregation/BeaconAggregationStrategy.h"

using namespace omnetpp;

class MECBeaconApp : public simu5g::MecAppBase
{

    inet::UdpSocket ueSocket;
    int localUePort;

    std::string subId;
    std::string aggregationStrategy;

    // Customizable parameters
    BeaconAggregationStrategy *strategy;
    simtime_t period_;

    inet::L3Address ueAppAddress;
    int ueAppPort;

    inet::TcpSocket* serviceSocket_;
    inet::TcpSocket* mp1Socket_;

    simu5g::HttpBaseMessage* mp1HttpMessage;
    simu5g::HttpBaseMessage* serviceHttpMessage;

    // State Recording
    cOutVector stateRecorderVector;

    protected:
        virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual void finish() override;

        virtual void handleProcessedMessage(omnetpp::cMessage *msg) override;
        virtual void handleServiceMessage(int connId) override;
        virtual void handleMp1Message(int connId) override;
        virtual void handleHttpMessage(int connId) override;

        virtual void handleUeMessage(omnetpp::cMessage *msg) override;

        virtual void handleSelfMessage(cMessage *msg) override;

        virtual void established(int connId) override;

    private:
        void sendBeaconToService(const SimpleMECBeacon&);
        void retrieveAllBeaconsFromService();
};

#endif
