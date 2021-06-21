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

#include "../emulation/NodeLocationExporter.h"

#include "inet/networklayer/common/L3AddressResolver.h"
#include "crownet/applications/beacon/Beacon_m.h"
#include "crownet/aid/AidHeader_m.h"

#include <time.h>
#include <sstream>

Define_Module(crownet::NodeLocationExporter);


crownet::NodeLocationExporter::~NodeLocationExporter()
{
    cancelAndDelete(selfMsg);
}

void crownet::NodeLocationExporter::initialize()
{
    startTime = par("startTime");
    stopTime = par("stopTime");
    port = par("port");
    address = par("address");
    xOffset = par("xOffset");
    yOffset = par("yOffset");
    interval = par("interval").doubleValue();

    selfMsg = new cMessage("NodeLocationExporter");

    mobilityModule = check_and_cast<IPositionHistoryProvider*>(
            getParentModule()->getParentModule()->getSubmodule("mobility"));
}

void crownet::NodeLocationExporter::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        switch (msg->getKind()) {
            case START:
                processStart();
                selfMsg->setKind(UPDATE);
                scheduleAfter(interval, selfMsg);
                break;

            case UPDATE:
                selfMsg->setKind(UPDATE);
                scheduleAfter(interval, selfMsg);

                sendLocationPacket();
                break;
        }
    } else if (msg->arrivedOn("socketInExternal")) {
        socketExt.processMessage(msg);
    }
}

void crownet::NodeLocationExporter::handleMessageWhenUp(cMessage *msg) {};
void crownet::NodeLocationExporter::finish() {};


void crownet::NodeLocationExporter::processStart()
{
    // External socket
    socketExt.setOutputGate(gate("socketOutExternal"));
    socketExt.bind(port);
}

void crownet::NodeLocationExporter::processStop() {
    socketExt.close();
}

void crownet::NodeLocationExporter::handleStartOperation(LifecycleOperation *operation)
{
    simtime_t start = std::max(startTime, simTime());

    if ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime)) {
        selfMsg->setKind(START);
        scheduleAt(start, selfMsg);
    }
}

void crownet::NodeLocationExporter::handleStopOperation(LifecycleOperation *operation) {}
void crownet::NodeLocationExporter::handleCrashOperation(LifecycleOperation *operation) {}

void crownet::NodeLocationExporter::sendLocationPacket() {
    Packet *packet = new Packet("Outbound");

    // Position
    int x = static_cast<int>(mobilityModule->getCurrentPosition().getX()) + xOffset;
    int y = yOffset - static_cast<int>(mobilityModule->getCurrentPosition().getY());
    int z = static_cast<int>(mobilityModule->getCurrentPosition().getZ());

    // Only send messages if the position changed.
    if (x == lastX && y == lastY) return;

    lastX = x;
    lastY = y;

    std::ostringstream locationMessageBuilder;

    // Position
    locationMessageBuilder << x << ";";
    locationMessageBuilder << y << ";";
    locationMessageBuilder << z << ";";

    // Velocity
    locationMessageBuilder << mobilityModule->getCurrentVelocity().getX() << ";";
    locationMessageBuilder << mobilityModule->getCurrentVelocity().getY() << ";";
    locationMessageBuilder << mobilityModule->getCurrentVelocity().getZ() << ";";

    std::string message = locationMessageBuilder.str();

    auto rawBytesData = inet::makeShared<inet::BytesChunk>();

    const uint8_t* messageBuffer = reinterpret_cast<const uint8_t*>(message.c_str());
    rawBytesData->copyFromBuffer(messageBuffer, message.length());
    packet->insertAtBack(rawBytesData);

    socketExt.sendTo(packet, inet::L3AddressResolver().resolve(address), port);
}
