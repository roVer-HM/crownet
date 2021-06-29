
#include "InboundEmulation.h"

#include "inet/networklayer/common/L3AddressResolver.h"
#include "crownet/applications/beacon/Beacon_m.h"
#include "crownet/aid/AidHeader_m.h"

#include <time.h>

Define_Module(crownet::InboundEmulation);

using com::example::peopledensitymeasurementprototype::model::proto::LocationMessageWrapper;
using com::example::peopledensitymeasurementprototype::model::proto::SingleLocationData;

crownet::InboundEmulation::~InboundEmulation()
{
    cancelAndDelete(selfMsg);
}

void crownet::InboundEmulation::initialize()
{
    startTime = par("startTime");
    stopTime = par("stopTime");
    externalPort = par("externalPort");
    externalAddress = par("externalAddress").stringValue();
    offsetNorthing = par("offsetNorthing");
    offsetEasting = par("offsetEasting");
    deviceId = par("deviceId");

    selfMsg = new cMessage("InboundEmulation");

    positionEmulator = check_and_cast<IPositionEmulator*>(
                getParentModule()->getParentModule()->getSubmodule("mobility"));
}

void crownet::InboundEmulation::handleMessage(cMessage *msg)
{

    if (msg->isSelfMessage()) {
        switch (msg->getKind()) {
            case START:
                processStart();
                break;
        }
    } else if (msg->arrivedOn("socketInExternal")) {
        socketExt.processMessage(msg);
    }
}

void crownet::InboundEmulation::handleMessageWhenUp(cMessage *msg) {};
void crownet::InboundEmulation::finish() {};

void crownet::InboundEmulation::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    EV_INFO << "Received data on socket" << endl;

    auto data = packet->peekAllAsBytes();
    const uint8_t* bytes = data->getBytes().data();

    std::string stringData(reinterpret_cast<char const*>(bytes)) ;

    LocationMessageWrapper wrapper;
    wrapper.ParseFromString(stringData);

    if (wrapper.has_single()) {
        SingleLocationData single = wrapper.single();

        int deviceId = single.deviceid();
        EV_INFO << "Received single location data from device" << deviceId << endl;

        //TODO: Check deviceId
        int northing = single.northing();
        int easting = single.easting();

        double y = -static_cast<double>(northing - offsetNorthing);
        double x = static_cast<double>(easting - offsetEasting);

        int bearing = single.bearing();
        int speed = single.speed();

        EV_INFO << "Move node to" << x << ":" << y << endl;

        positionEmulator->injectEmulatedPosition(
                inet::Coord{x, y},
                artery::Angle::from_degree(bearing),
                1.0);
    }

    delete packet;
}

void crownet::InboundEmulation::socketErrorArrived(UdpSocket *socket, Indication *indication) {
    EV_INFO << "Socket error." << endl;
}

void crownet::InboundEmulation::socketClosed(UdpSocket *socket) {}


void crownet::InboundEmulation::processStart()
{
    // External socket
    socketExt.setOutputGate(gate("socketOutExternal"));
    socketExt.bind(inet::L3AddressResolver().resolve("0.0.0.0"), externalPort);
    socketExt.setCallback(this);

    positionEmulator->useEmulatedPositionSource(true);
}

void crownet::InboundEmulation::processStop() {
    socketExt.close();
}

void crownet::InboundEmulation::handleStartOperation(LifecycleOperation *operation)
{
    simtime_t start = std::max(startTime, simTime());

    if ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime)) {
        selfMsg->setKind(START);
        scheduleAt(start, selfMsg);
    }
}

void crownet::InboundEmulation::handleStopOperation(LifecycleOperation *operation) {}
void crownet::InboundEmulation::handleCrashOperation(LifecycleOperation *operation) {}
