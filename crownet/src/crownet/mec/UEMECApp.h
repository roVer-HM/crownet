#include <omnetpp.h>
#include <common/binder/Binder.h>
#include <vector>
#include <map>
#include "inet/mobility/contract/IMobility.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "packets/SimpleMECBeacon_m.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/common/TimeTag_m.h"

#ifndef UEMECAPP_H
#define UEMECAPP_H

using namespace omnetpp;

class UEMECApp : public cSimpleModule {
private:
    typedef inet::Ptr<SimpleMECBeacon> _beacon;

    struct NodeMobility {
        int nodeId;
        const char *name;

        inet::IMobility *mobility;
    };

    struct BeaconPosition {
        double x;
        double y;

        enum {
            D2D, MEC
        } source;
    };

    std::map<int, BeaconPosition> beaconPositions;

    inet::IMobility *mobility;

    inet::UdpSocket socketMEC;
    inet::UdpSocket socketD2D;

    int localPortMEC;
    int localPortD2D;
    int deviceAppPort;
    int mecAppPort;

    inet::L3Address deviceAppAddress;
    inet::L3Address mecAppAddress;
    inet::L3Address destAddressD2D;

    std::string mecAppName;

    int ueId;
    int beaconType;

    const char *personNode;
    const char *vehicleNode;

    simtime_t period;
    simtime_t evalPeriod;

    bool disseminateMECBeacons;
    bool disseminateD2DBeacons;

    cMessage *selfStart;
    cMessage *selfStop;
    cMessage *selfBeacon;
    cMessage *selfEval;

    cOutVector vBeaconX;
    cOutVector vBeaconY;
    cOutVector vRealX;
    cOutVector vRealY;
    cOutVector vNodeId;

    cOutVector rcvBeaconMEC;
    cOutVector rcvBeaconD2D;


public:
    UEMECApp();
    virtual ~UEMECApp();

protected:
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void initialize(int) override;
    virtual void handleMessage(cMessage *msg) override;

private:
    virtual void handleBeaconsFromMEC(const ResponseBeaconMessage*);
    virtual void handleBeaconFromD2D(const SimpleMECBeacon*);
    virtual void mecDeviceAppAck(cMessage*);
    virtual _beacon generateBeacon();
    virtual void sendBeaconMEC(_beacon);
    virtual void sendBeaconD2D(_beacon);
    virtual void disseminateBeacon();
    virtual void mecAppSendStart();
    virtual void evaluate();
    virtual std::vector<NodeMobility> getMobilityBySubmoduleVector(const char*);

};

#endif
