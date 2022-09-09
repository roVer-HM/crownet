#include "BeaconService.h"
#include "BeaconJSON.h"

Define_Module(BeaconService);

BeaconService::BeaconService()
{

}

BeaconService::~BeaconService()
{

}

void BeaconService::initialize(int stage)
{
    MecServiceBase::initialize(stage);

    if (stage != inet::INITSTAGE_APPLICATION_LAYER) return;
}

void BeaconService::finish()
{

}

void BeaconService::handleMessage(cMessage* msg)
{
    MecServiceBase::handleMessage(msg);
}

void BeaconService::handleGETRequest(const HttpRequestMessage* msg, inet::TcpSocket* socket)
{
    std::string uri = msg->getUri();

    if(uri.compare(baseUri + "/all") == 0)
    {
        Http::send200Response(socket, beacon_json::serializeBeacons(beaconRepository.getAll()).dump().c_str());
    }

}

void BeaconService::handlePUTRequest(const HttpRequestMessage* msg, inet::TcpSocket* socket)
{

}

void BeaconService::handlePOSTRequest(const HttpRequestMessage* msg, inet::TcpSocket* socket)
{
    std::string uri = msg->getUri();
    std::string body = msg->getBody();

    if(uri.compare(baseUri + "/beacon") == 0)
    {
        EV << "Receive beacon: " << body << std::endl;
        beaconRepository.add(beacon_json::parseBeaconString(body));
    }
    else
    {
        EV << "Can't handle POST request for URI " << uri << std::endl;
    }
}

void BeaconService::handleDELETERequest(const HttpRequestMessage* msg, inet::TcpSocket* socket)
{

}
