#include "UEMECApp.h"

#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include <cstdlib>

#include <apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_m.h>
#include <apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_Types.h>

using namespace inet;

Define_Module(UEMECApp);

UEMECApp::UEMECApp()
{

}

UEMECApp::~UEMECApp()
{

}

void UEMECApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage != inet::INITSTAGE_APPLICATION_LAYER) return;

    vBeaconX.setName("beaconX");
    vBeaconY.setName("beaconY");
    vRealX.setName("realX");
    vRealY.setName("realY");
    vNodeId.setName("nodeId");

    rcvBeaconMEC.setName("receivedMEC");
    rcvBeaconD2D.setName("receivedD2D");

    period = par("period");
    evalPeriod = par("evalPeriod");

    localPortMEC = par("localPortMEC");
    localPortD2D = par("localPortD2D");
    deviceAppPort = par("deviceAppPort");

    char *deviceSimbolicAppAddress = (char*)par("deviceAppAddress").stringValue();
    deviceAppAddress = inet::L3AddressResolver().resolve(deviceSimbolicAppAddress);

    destAddressD2D = inet::L3AddressResolver().resolve(par("destAddressD2D"));

    mobility = check_and_cast<inet::IMobility*>(getParentModule()->getSubmodule("mobility"));

    // Beacon parameters
    beaconType = par("beaconType").intValue();
    ueId = getParentModule()->getId();
    mecAppName = par("mecAppName").stringValue();

    // Socket MEC
    socketMEC.setOutputGate(gate("socketOut"));
    socketMEC.bind(deviceAppAddress, localPortMEC);

    // Socket D2D
    const char *multicastInterface = par("multicastInterface");
    inet::IInterfaceTable *ift = inet::getModuleFromPar<inet::IInterfaceTable >(par("interfaceTableModule"), this);
    inet::MulticastGroupList mgl = ift->collectMulticastGroups();
    inet::NetworkInterface *ie = ift->findInterfaceByName(multicastInterface);

    socketD2D.setOutputGate(gate("socketOut"));
    socketD2D.joinLocalMulticastGroups(mgl);
    socketD2D.setMulticastOutputInterface(ie->getInterfaceId());

    disseminateMECBeacons = par("disseminateMECBeacons").boolValue();
    disseminateD2DBeacons = par("disseminateD2DBeacons").boolValue();

    personNode = par("personNode");
    vehicleNode = par("vehicleNode");

    simtime_t startTime = par("startTime");
    scheduleAt(simTime() + startTime, new cMessage("selfStart"));
    scheduleAt(simTime() + evalPeriod, new cMessage("selfEval"));
}

void UEMECApp::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        handleSelfMessage(msg);
    } else {
        if (socketMEC.belongsToSocket(msg)) {
            inet::Packet *packet = check_and_cast<inet::Packet*>(msg);
            inet::L3Address ipAdd =
                    packet->getTag<L3AddressInd>()->getSrcAddress();

            if (ipAdd == deviceAppAddress
                    || ipAdd == inet::L3Address("127.0.0.1")
                    || ipAdd == inet::L3Address("0.0.0.0")) {
                auto deviceAppPacket = packet->peekAtFront<DeviceAppPacket>();
                if (strcmp(deviceAppPacket->getType(), ACK_START_MECAPP) == 0)
                    mecDeviceAppAck(msg);
            } else {
                auto responseBeaconMessage = packet->peekAtFront<
                        ResponseBeaconMessage>();
                handleBeaconsFromMEC(responseBeaconMessage.get());
            }
        } else if (socketD2D.belongsToSocket(msg)) {
            inet::Packet *pkt = check_and_cast<inet::Packet*>(msg);
            auto beacon = pkt->peekAtFront<SimpleMECBeacon>();

            handleBeaconFromD2D(beacon.get());
        } else {
            throw cRuntimeError("UEMECApp::handleMessage - Cannot handle %s",
                    msg->getFullName());
        }

        delete msg;
    }
}


void UEMECApp::handleSelfMessage(cMessage *msg) {
    if (std::strcmp(msg->getName(), "selfStart") == 0) {
        socketD2D.bind(localPortD2D);
        mecAppSendStart();
    } else if (std::strcmp(msg->getName(), "selfEval") == 0) {
        evaluate();
        scheduleAt(simTime() + evalPeriod, new cMessage("selfEval"));
    } else if (std::strcmp(msg->getName(), "selfBeacon") == 0) {
        disseminateBeacon();
        scheduleAt(simTime() + period, new cMessage("selfBeacon"));
    } else
    {
        throw cRuntimeError("UEMECApp::handleSelfMessage - Cannot handle %s", msg->getFullName());
    }

    delete msg;
}



void UEMECApp::mecAppSendStart()
{
    inet::Packet* pkt = new inet::Packet("WarningAlertPacketStart");

    auto start = inet::makeShared<DeviceAppStartPacket>();
    start->setType(START_MECAPP);
    start->setMecAppName(mecAppName.c_str());

    start->setChunkLength(inet::B(2+mecAppName.size()+1));
    start->addTagIfAbsent<inet::CreationTimeTag>()->setCreationTime(simTime());

    pkt->insertAtBack(start);
    socketMEC.sendTo(pkt, deviceAppAddress, deviceAppPort);

    EV << "Sent Start to Device App!" << std::endl;
}

void UEMECApp::mecDeviceAppAck(cMessage *msg)
{
    inet::Packet* packet = check_and_cast<inet::Packet*>(msg);
    auto pkt = packet->peekAtFront<DeviceAppStartAckPacket>();

    if(pkt->getResult() == true)
    {
        mecAppAddress = L3AddressResolver().resolve(pkt->getIpAddress());
        mecAppPort = pkt->getPort();
        EV << "Received Start Ack from Device App!" << std::endl;

        scheduleAt(simTime() + 1, new cMessage("selfBeacon"));
    }
    else
    {
        EV << "Start Device App failed!" << pkt->getReason() << std::endl;
    }
}

void UEMECApp::handleBeaconsFromMEC(const ResponseBeaconMessage *beacons)
{
    EV << "Received beacons from MEC" << std::endl;

    for (int i = 0; i < beacons->getBeaconsArraySize(); ++i)
    {
        ReducedMECBeacon rb = beacons->getBeacons(i);

        rcvBeaconMEC.record(rb.getId());

        EV  << rb.getId() << ","
            << rb.getNodeType() << ","
            << rb.getXPos() << ","
            << rb.getYPos() << std::endl;

        beaconPositions[rb.getId()] = { rb.getXPos(), rb.getYPos(), BeaconPosition::MEC };
    }
}

void UEMECApp::handleBeaconFromD2D(const SimpleMECBeacon *beacon)
{
    EV  << "Received beacon from D2D" << std::endl;
    EV  << beacon->getNodeId() << ","
        << beacon->getNodeType() << ","
        << beacon->getXPos() << ","
        << beacon->getYPos() << std::endl;

    rcvBeaconD2D.record(beacon->getNodeId());

    beaconPositions[beacon->getNodeId()] ={ beacon->getXPos(), beacon->getYPos(), BeaconPosition::D2D };
}

UEMECApp::_beacon UEMECApp::generateBeacon()
{
    _beacon beacon = inet::makeShared<SimpleMECBeacon>();

    beacon->setNodeId(ueId);
    beacon->setNodeType(beaconType);
    beacon->setXPos(mobility->getCurrentPosition().x);
    beacon->setYPos(mobility->getCurrentPosition().y);
    beacon->setChunkLength(inet::B(400));
    beacon->addTagIfAbsent<inet::CreationTimeTag>()->setCreationTime(simTime());
    beacon->setAdditionalDataArraySize(100);

    for (int i = 0; i < 100; i++) beacon->setAdditionalData(i, std::rand());

    return beacon;
}

void UEMECApp::sendBeaconMEC(_beacon beacon)
{
    inet::Packet* pkt = new inet::Packet("SimpleMECBeacon");
    pkt->insertAtBack(beacon);
    socketMEC.sendTo(pkt, mecAppAddress, mecAppPort);
}

void UEMECApp::sendBeaconD2D(_beacon beacon)
{
    inet::Packet* pkt = new inet::Packet("SimpleMECBeaconD2D");
    pkt->insertAtBack(beacon);
    socketD2D.sendTo(pkt, destAddressD2D, localPortD2D);
}

void UEMECApp::disseminateBeacon()
{
    EV << "Disseminate beacon" << std::endl;

    _beacon beacon = generateBeacon();

    if (disseminateMECBeacons)
        sendBeaconMEC(beacon);

    if (disseminateD2DBeacons)
        sendBeaconD2D(beacon);
}

void UEMECApp::evaluate()
{
    auto vNodeMobilities = getMobilityBySubmoduleVector(vehicleNode);
    auto pNodeMobilities = getMobilityBySubmoduleVector(personNode);

    std::vector<NodeMobility> mobilities;
    mobilities.insert(mobilities.end(), vNodeMobilities.begin(), vNodeMobilities.end());
    mobilities.insert(mobilities.end(), pNodeMobilities.begin(), pNodeMobilities.end());

    EV << "Evaluate - found mobility info for nodes: " << mobilities.size() << std::endl;

    for (auto& mob : mobilities)
    {

        if (beaconPositions.count(mob.nodeId))
        {
            vNodeId.record(mob.nodeId);
            EV << "Node: " << mob.nodeId << " " << std::string(mob.name) << std::endl;

            vRealX.record(mob.mobility->getCurrentPosition().x);
            vRealY.record(mob.mobility->getCurrentPosition().y);

            auto bpos = beaconPositions[mob.nodeId];

            vBeaconX.record(bpos.x);
            vBeaconY.record(bpos.y);
        }
    }
}

std::vector<UEMECApp::NodeMobility> UEMECApp::getMobilityBySubmoduleVector(const char* name)
{
    std::vector<NodeMobility> mobilities;

    cModule *nodes = getSystemModule()->getSubmodule(name, 0);

    if (nodes != nullptr)
    {
        for (int i = 0; i < nodes->getVectorSize(); ++i)
        {
            cModule *node = getSystemModule()->getSubmodule(name, i);

            if (node != nullptr)
            {
                cModule *mobilitySubmodule = node->getSubmodule("mobility");

                if (mobilitySubmodule != nullptr)
                {
                    inet::IMobility *mob = check_and_cast<inet::IMobility*>(mobilitySubmodule);

                    const char *nodeName = node->getDisplayName();

                    if (nodeName == nullptr)
                        nodeName = node->getName();

                    NodeMobility nodeMobility;
                    nodeMobility.nodeId = node->getId();
                    nodeMobility.mobility = mob;
                    nodeMobility.name = nodeName;

                    mobilities.push_back(nodeMobility);
                }
            }
        }
    }

    return mobilities;
}

