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

#include "UEBeaconApp.h"

using namespace inet;
using namespace simu5g;

Define_Module(UEBeaconApp);

void UEBeaconApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage != inet::INITSTAGE_APPLICATION_LAYER) return;

    selfStart_ = new cMessage("selfStart");
    selfStop_ = new cMessage("selfStop");
    selfBeacon_ = new cMessage("selfBeacon");

    beaconArrivalVector.setName("BeaconArrival");

    period_ = par("period");
    localPort_ = par("localPort");
    deviceAppPort_ = par("deviceAppPort");

    beaconType = par("beaconType").intValue();

    sourceSimbolicAddress = (char*)getParentModule()->getFullName();
    deviceSimbolicAppAddress_ = (char*)par("deviceAppAddress").stringValue();
    deviceAppAddress_ = inet::L3AddressResolver().resolve(deviceSimbolicAppAddress_);

    cModule *temp = getParentModule()->getSubmodule("mobility");
    if(temp != NULL) mobility = check_and_cast<inet::IMobility*>(temp);

    ueId = getParentModule()->getId();

    mecAppName = par("mecAppName").stringValue();

    socket.setOutputGate(gate("socketOut"));
    socket.bind(deviceAppAddress_, localPort_);

    simtime_t startTime = par("startTime");
    scheduleAt(simTime() + startTime, selfStart_);
}

void UEBeaconApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        if (std::strncmp(msg->getName(), "selfStart", 9) == 0)
        {
            EV << "Start MECBeaconApp" << std::endl;

            sendStart();
        }
        else if (std::strncmp(msg->getName(), "selfBeacon", 10) == 0)
        {
            EV << "Send Beacon" << std::endl;

            sendBeacon();
            scheduleAt(simTime() + period_, selfBeacon_);
        }
    }
    else
    {
        inet::Packet* packet = check_and_cast<inet::Packet*>(msg);
        inet::L3Address ipAdd = packet->getTag<L3AddressInd>()->getSrcAddress();

        if (ipAdd == deviceAppAddress_ || ipAdd == inet::L3Address("127.0.0.1") || ipAdd == inet::L3Address("0.0.0.0"))
        // Message from device app
        {
            auto deviceAppPacket = packet->peekAtFront<DeviceAppPacket>();

            if(strcmp(deviceAppPacket->getType(), ACK_START_MECAPP) == 0)
            {
                startAck(msg);
            }
        }
        else
        // From MEC App
        {
            auto responseBeaconMessage = packet->peekAtFront<ResponseBeaconMessage>();
            handleBeacons(responseBeaconMessage.get());
        }

        delete msg;
    }

}

void UEBeaconApp::handleBeacons(const ResponseBeaconMessage* rbm)
{
    // Do something with beacons from MEC
    // Just print for now

    EV << "============= Received Beacons:" << std::endl;

    for (int i = 0; i < rbm->getBeaconsArraySize(); ++i)
    {
        ReducedMECBeacon rb = rbm->getBeacons(i);
        EV << rb.getXPos() << "," << rb.getYPos() << std::endl;

        if (ueId != rb.getId())
            beaconArrivalVector.record(rb.getId());
    }
}

void UEBeaconApp::startAck(cMessage *msg)
{
    inet::Packet* packet = check_and_cast<inet::Packet*>(msg);
    auto pkt = packet->peekAtFront<DeviceAppStartAckPacket>();

    if(pkt->getResult() == true)
    {
        mecAppAddress_ = L3AddressResolver().resolve(pkt->getIpAddress());
        mecAppPort_ = pkt->getPort();

        EV << "Received Start Ack from Device App!" << std::endl;

        scheduleAt(simTime(), selfBeacon_);
    }
    else
    {
        EV << "Start Device App failed!" << pkt->getReason() << std::endl;
    }
}

void UEBeaconApp::sendStart()
{
    inet::Packet* pkt = new inet::Packet("WarningAlertPacketStart");

    auto start = inet::makeShared<DeviceAppStartPacket>();
    start->setType(START_MECAPP);
    start->setMecAppName(mecAppName.c_str());

    start->setChunkLength(inet::B(2+mecAppName.size()+1));
    start->addTagIfAbsent<inet::CreationTimeTag>()->setCreationTime(simTime());

    pkt->insertAtBack(start);
    socket.sendTo(pkt, deviceAppAddress_, deviceAppPort_);

    EV << "Sent Start to Device App!" << std::endl;
}

void UEBeaconApp::sendBeacon()
{
    inet::Packet* pkt = new inet::Packet("SimpleMECBeacon");
    auto beacon = inet::makeShared<SimpleMECBeacon>();

    beacon->setNodeId(ueId);
    beacon->setNodeType(beaconType);
    beacon->setXPos(mobility->getCurrentPosition().x);
    beacon->setYPos(mobility->getCurrentPosition().y);

    beacon->setChunkLength(inet::B(20));
    beacon->addTagIfAbsent<inet::CreationTimeTag>()->setCreationTime(simTime());
    pkt->insertAtBack(beacon);

    socket.sendTo(pkt, mecAppAddress_ , mecAppPort_);
}
