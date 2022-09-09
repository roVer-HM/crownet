
#include "MECBeaconApp.h"

#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/Packet_m.h"

#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

#include "nodes/mec/utils/httpUtils/httpUtils.h"
#include "nodes/mec/utils/httpUtils/json.hpp"
#include "nodes/mec/MECPlatform/MECServices/packets/HttpResponseMessage/HttpResponseMessage.h"

#include "inet/transportlayer/contract/tcp/TcpSocket.h"

#include "BeaconJSON.h"

#include <vector>

Define_Module(MECBeaconApp);

using namespace inet;
using namespace omnetpp;


void MECBeaconApp::initialize(int stage)
{
    MecAppBase::initialize(stage);

    if (stage!=inet::INITSTAGE_APPLICATION_LAYER)return;

    ueSocket.setOutputGate(gate("socketOut"));

    localUePort = par("localUePort");
    ueSocket.bind(localUePort);

    aggregationStrategy = par("aggregationStrategy").stringValue();

    if (aggregationStrategy == "default")
    {
        strategy = new DefaultBeaconAggregationStrategy();
    }
    else if (aggregationStrategy == "dz")
    {
        strategy = new DangerZoneBeaconFilter();
        ((DangerZoneBeaconFilter*) strategy)->addDangerZone(350, 470, 20);
    }
    else if (aggregationStrategy == "prio")
    {
        strategy = new PriorityBasedAggregator(
                par("prioAggregationGroups").intValue(),
                par("prioAggregationSize").intValue());
    } else if (aggregationStrategy == "v2p") {
        strategy = new V2P_P2VFilter();
    }

    period_ = par("period");

    stateRecorderVector.setName("stateRecorder");
    stateRecorderVector.record(0);

    scheduleAt(simTime() + 0.0001, new cMessage("connectMp1"));
}

void MECBeaconApp::handleMessage(cMessage *msg)
{
    if (!msg->isSelfMessage())
    {
        if(ueSocket.belongsToSocket(msg))
        {
            handleUeMessage(msg);

            delete msg;
            return;
        }
    }
    else
    {
        if (strcmp(msg->getName(), "sendBeacons") == 0)
        {
            retrieveAllBeaconsFromService();

            scheduleAt(simTime() + strategy->getInterval(period_), new cMessage("sendBeacons"));
            return;
        }
    }
    MecAppBase::handleMessage(msg);

}

void MECBeaconApp::finish()
{
    MecAppBase::finish();
}

void MECBeaconApp::handleUeMessage(omnetpp::cMessage *msg)
{
    stateRecorderVector.record(1);
    auto pk = check_and_cast<Packet *>(msg);

    ueAppAddress = pk->getTag<L3AddressInd>()->getSrcAddress();
    ueAppPort = pk->getTag<L4PortInd>()->getSrcPort();

    auto mecPk = pk->peekAtFront<SimpleMECBeacon>();
    SimpleMECBeacon beacon = *mecPk.get();

    EV << "Received Beacon " << mecPk->getNodeId() << " " << mecPk->getXPos() << " " << mecPk->getYPos() << std::endl;

    if (strategy->handleUEBeacon(beacon)) {
        sendBeaconToService(beacon);
    }
}

void MECBeaconApp::sendBeaconToService(const SimpleMECBeacon& beacon)
{
    if (serviceSocket_.getState() != TcpSocket::CONNECTED)
    {
        EV << "Tried to send beacon to service but socket is not connected" << std::endl;
        return;
    }

    // Construct beacon JSON
    nlohmann::json beaconJson = beacon_json::serializeBeacon(beacon);

    std::string uri = "/example/Beacon/v1/beacon";
    std::string host = serviceSocket_.getRemoteAddress().str()+":"+std::to_string(serviceSocket_.getRemotePort());

    Http::sendPostRequest(&serviceSocket_, beaconJson.dump().c_str(), host.c_str(), uri.c_str());
}

void MECBeaconApp::retrieveAllBeaconsFromService()
{
    if (serviceSocket_.getState() != TcpSocket::CONNECTED)
    {
        EV << "Tried to send beacon to service but socket is not connected" << std::endl;
        return;
    }

    stateRecorderVector.record(5);

    std::string uri = "/example/Beacon/v1/all";
    std::string host = serviceSocket_.getRemoteAddress().str()+":"+std::to_string(serviceSocket_.getRemotePort());

    Http::sendGetRequest(&serviceSocket_, host.c_str(), uri.c_str(), "", "");
}

void MECBeaconApp::established(int connId)
{
   if(connId == mp1Socket_.getSocketId())
   {
       stateRecorderVector.record(2);
       EV << "Mp1Socket Connected"<< endl;
       // get endPoint of the required service
       const char *uri = "/example/mec_service_mgmt/v1/services?ser_name=BeaconService";

       std::string host = mp1Socket_.getRemoteAddress().str()+":"+std::to_string(mp1Socket_.getRemotePort());

       Http::sendGetRequest(&mp1Socket_, host.c_str(), uri);
   }
   else if (connId == serviceSocket_.getSocketId())
   {
       EV << "Service Socket message"<< endl;
   }
   else
   {
       throw cRuntimeError("MecAppBase::socketEstablished - Socket %d not recognized", connId);
   }
}

void MECBeaconApp::handleMp1Message()
{
    EV << "MP1 Message" << std::endl;

    nlohmann::json jsonBody = nlohmann::json::parse(mp1HttpMessage->getBody()); // get the JSON structure
    if(!jsonBody.empty())
    {
        jsonBody = jsonBody[0];
        std::string serName = jsonBody["serName"];
        if(serName.compare("BeaconService") == 0)
        {
            if(jsonBody.contains("transportInfo"))
            {
                nlohmann::json endPoint = jsonBody["transportInfo"]["endPoint"]["addresses"];
                EV << "address: " << endPoint["host"] << " port: " <<  endPoint["port"] << endl;
                std::string address = endPoint["host"];
                serviceAddress = L3AddressResolver().resolve(address.c_str());;
                servicePort = endPoint["port"];

                // Start Service
                stateRecorderVector.record(3);
                scheduleAt(simTime() + 0.001, new cMessage("connectService"));
            }
        }
        else
        {
            EV << "MECWarningAlertApp::handleMp1Message - BeaconService not found"<< endl;
            serviceAddress = L3Address();
        }
    }
}

void MECBeaconApp::handleServiceMessage()
{
    if(serviceHttpMessage->getType() == RESPONSE)
    {

        if (ueAppAddress.isUnspecified())
        {
            EV << "Received service data but has no UE address yet" << std::endl;
            return;
        }

        stateRecorderVector.record(4);

        std::vector<SimpleMECBeacon> beacons = beacon_json::parseBeaconsString(serviceHttpMessage->getBody());

        EV << "Received " << beacons.size() << " beacons from service" << std::endl;

        // That's where the magic happens
        Packet *pkt = strategy->createBeaconPacket(beacons);

        if (pkt != nullptr)
            ueSocket.sendTo(pkt, ueAppAddress, ueAppPort);
    }
}

void MECBeaconApp::handleSelfMessage(cMessage *msg)
{
    EV << "MecAppBase::handleMessage " << msg->getName() << endl;

    if(strcmp(msg->getName(), "connectMp1") == 0)
    {
        connect(&mp1Socket_, mp1Address, mp1Port);
    }
    else if(strcmp(msg->getName(), "connectService") == 0)
    {
        if(!serviceAddress.isUnspecified() && serviceSocket_.getState() != inet::TcpSocket::CONNECTED)
        {
            EV << "Connect to Service" << std::endl;
            connect(&serviceSocket_, serviceAddress, servicePort);

            // Start beacons
            scheduleAt(simTime() + 0.002, new cMessage("sendBeacons"));
        }
        else
        {
            EV << "Connect to Service:" << serviceAddress << " " << serviceSocket_.getState() << std::endl;
        }

    }

    delete msg;
}
