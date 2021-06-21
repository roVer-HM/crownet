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

#include "../emulation/DensityMessageHandler.h"

#include "inet/networklayer/common/L3AddressResolver.h"
#include "crownet/applications/beacon/Beacon_m.h"
#include "crownet/aid/AidHeader_m.h"

#include <time.h>

Define_Module(crownet::DensityMessageHandler);

using com::example::peopledensitymeasurementprototype::model::proto::LocationMessageWrapper;
using com::example::peopledensitymeasurementprototype::model::proto::SingleLocationData;

crownet::DensityMessageHandler::~DensityMessageHandler()
{
    cancelAndDelete(selfMsg);
}

void crownet::DensityMessageHandler::initialize()
{
    startTime = par("startTime");
    stopTime = par("stopTime");
    localPort = par("localPort");
    externalPort = par("externalPort");
    localAddress = par("localAddress").stringValue();
    externalAddress = par("externalAddress").stringValue();
    offsetNorthing = par("offsetNorthing");
    offsetEasting = par("offsetEasting");

    selfMsg = new cMessage("UdpMessageHandler");
}

void crownet::DensityMessageHandler::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        switch (msg->getKind()) {
            case START:
                processStart();
                break;
        }
    } else if (msg->arrivedOn("socketIn")) {
        socket.processMessage(msg);
    } else if (msg->arrivedOn("socketInExternal")) {
        socketExt.processMessage(msg);
    }
}

void crownet::DensityMessageHandler::handleMessageWhenUp(cMessage *msg) {};
void crownet::DensityMessageHandler::finish() {};

void crownet::DensityMessageHandler::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // Extract beacon
    auto header = packet->popAtFront<AidHeader>();
    auto beacon = packet->popAtFront<BeaconPacket>();

    EV_INFO << "Sent beacon for device " << beacon->getNodeId() << " at time " << time(NULL) << endl;

    int xpos = static_cast<int>(beacon->getPos().getX());
    int ypos = static_cast<int>(beacon->getPos().getY());

    int northing = offsetNorthing - (ypos - ypos % 5);
    int easting = offsetEasting + (xpos - xpos % 5);

    SingleLocationData *single = new SingleLocationData();
    single->set_deviceid(beacon->getNodeId());
    single->set_hemisphere(true);
    single->set_bearing(0);
    single->set_northing(northing);
    single->set_easting(easting);
    single->set_zoneid(32);
    single->set_accuracy(10);
    single->set_timestamp(time(NULL));
    single->set_ttl(30);

    sendSingleLocationBroadcast(socketExt, single);
    delete packet;
}

void crownet::DensityMessageHandler::socketErrorArrived(UdpSocket *socket, Indication *indication) {
    EV_INFO << "Socket error." << endl;
}

void crownet::DensityMessageHandler::socketClosed(UdpSocket *socket) {}

void crownet::DensityMessageHandler::sendSingleLocationBroadcast(UdpSocket socket, SingleLocationData *data)
{
    LocationMessageWrapper wrapper;
    wrapper.set_allocated_single(data);

    int size = wrapper.ByteSize();
    uint8_t *buffer = new uint8_t[size];

    wrapper.SerializeToArray(buffer, size);

    Packet *packet = new Packet("Outbound");

    auto rawBytesData = inet::makeShared<inet::BytesChunk>();
    rawBytesData->copyFromBuffer(buffer, size);
    packet->insertAtBack(rawBytesData);

    socket.sendTo(packet, inet::L3AddressResolver().resolve(externalAddress), externalPort);

    delete[] buffer;
}

void crownet::DensityMessageHandler::processStart()
{
    socket.setOutputGate(gate("socketOut"));
    MulticastGroupList mgl = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)->collectMulticastGroups();
    socket.joinLocalMulticastGroups(mgl);

    socket.bind(localPort);
    socket.setCallback(this);

    // External socket
    socketExt.setOutputGate(gate("socketOutExternal"));
    socketExt.bind(externalPort);
    socketExt.setCallback(this);
}

void crownet::DensityMessageHandler::processStop() {
    socket.close();
    socketExt.close();
}

void crownet::DensityMessageHandler::handleStartOperation(LifecycleOperation *operation)
{
    simtime_t start = std::max(startTime, simTime());

    if ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime)) {
        selfMsg->setKind(START);
        scheduleAt(start, selfMsg);
    }
}

void crownet::DensityMessageHandler::handleStopOperation(LifecycleOperation *operation) {}
void crownet::DensityMessageHandler::handleCrashOperation(LifecycleOperation *operation) {}
