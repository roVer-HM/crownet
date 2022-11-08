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

#include "UED2DBeaconApp.h"

#include "packets/SimpleMECBeacon_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/Packet_m.h"

Define_Module(UED2DBeaconApp);

UED2DBeaconApp::UED2DBeaconApp() {


}

UED2DBeaconApp::~UED2DBeaconApp() {
    // TODO Auto-generated destructor stub
}

void UED2DBeaconApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage != inet::INITSTAGE_APPLICATION_LAYER) return;

    localPort = par("localPort");
    destAddress_ = inet::L3AddressResolver().resolve(par("destAddress"));

    socket.setOutputGate(gate("socketOut"));

    beaconArrivalVector.setName("BeaconArrival");

    inet::IInterfaceTable *ift = inet::getModuleFromPar< inet::IInterfaceTable >(par("interfaceTableModule"), this);

    const char *multicastInterface = par("multicastInterface");

    inet::MulticastGroupList mgl = ift->collectMulticastGroups();
    socket.joinLocalMulticastGroups(mgl);

    if (multicastInterface[0]) {
        inet::NetworkInterface *ie = ift->findInterfaceByName(multicastInterface);
        if (!ie)
            throw cRuntimeError("Wrong multicastInterface setting: no interface named \"%s\"", multicastInterface);
        socket.setMulticastOutputInterface(ie->getInterfaceId());
    }

    cModule *temp = getParentModule()->getSubmodule("mobility");
    if(temp != NULL) mobility = check_and_cast<inet::IMobility*>(temp);

    ueId = getParentModule()->getId();

    period_ = par("period");

    simtime_t startTime = par("startTime");
    scheduleAt(simTime() + startTime, new cMessage("start"));
}


void UED2DBeaconApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage() && strcmp(msg->getName(), "start") == 0)
    {
        socket.bind(localPort);

        sendBeacon();
        scheduleAt(simTime() + period_, new cMessage("sendBeacon"));
    }
    else if (msg->isSelfMessage() && strcmp(msg->getName(), "sendBeacon") == 0)
    {
        sendBeacon();
        scheduleAt(simTime() + period_, new cMessage("sendBeacon"));
    }
    else
    {
        auto pk = check_and_cast<inet::Packet*>(msg);
        auto beacon = pk->peekAtFront<SimpleMECBeacon>();

        EV << "Received beacon from " << beacon->getNodeId() << std::endl;

        if (ueId != beacon->getNodeId())
            beaconArrivalVector.record(beacon->getNodeId());
    }
    delete msg;
}

void UED2DBeaconApp::sendBeacon()
{
    inet::Packet* pkt = new inet::Packet("SimpleMECBeacon");
    auto beacon = inet::makeShared<SimpleMECBeacon>();

    beacon->setNodeId(ueId);
    beacon->setXPos(mobility->getCurrentPosition().x);
    beacon->setYPos(mobility->getCurrentPosition().y);

    beacon->setChunkLength(inet::B(20));
    beacon->addTagIfAbsent<inet::CreationTimeTag>()->setCreationTime(simTime());
    pkt->insertAtBack(beacon);

    socket.sendTo(pkt, destAddress_ , localPort);
}
