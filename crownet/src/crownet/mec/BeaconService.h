
#ifndef _BEACONSERVICE_H
#define _BEACONSERVICE_H

#include <nodes/mec/MECPlatform/MECServices/MECServiceBase/MecServiceBase.h>
#include "BeaconRepository.h"

class BeaconService: public simu5g::MecServiceBase
{
  private:
    std::string baseUri = "/example/Beacon/v1";

    BeaconRepository beaconRepository;

  public:
    BeaconService();

  protected:

    virtual void initialize(int) override;
    virtual void finish() override;
    virtual void handleMessage(inet::cMessage*) override;

    virtual void handleGETRequest(const simu5g::HttpRequestMessage*, inet::TcpSocket*) override;
    virtual void handlePOSTRequest(const simu5g::HttpRequestMessage*, inet::TcpSocket*) override;
    virtual void handlePUTRequest(const simu5g::HttpRequestMessage*, inet::TcpSocket*) override;
    virtual void handleDELETERequest(const simu5g::HttpRequestMessage*, inet::TcpSocket*) override;

    virtual ~BeaconService();
};

#endif

