
#ifndef _BEACONSERVICE_H
#define _BEACONSERVICE_H

#include <nodes/mec/MECPlatform/MECServices/MECServiceBase/MecServiceBase.h>
#include "BeaconRepository.h"

class BeaconService: public MecServiceBase
{
  private:
    std::string baseUri = "/example/Beacon/v1";

    BeaconRepository beaconRepository;

  public:
    BeaconService();

  protected:

    virtual void initialize(int) override;
    virtual void finish() override;
    virtual void handleMessage(cMessage*) override;

    virtual void handleGETRequest(const HttpRequestMessage*, inet::TcpSocket*) override;
    virtual void handlePOSTRequest(const HttpRequestMessage*, inet::TcpSocket*) override;
    virtual void handlePUTRequest(const HttpRequestMessage*, inet::TcpSocket*) override;
    virtual void handleDELETERequest(const HttpRequestMessage*, inet::TcpSocket*) override;

    virtual ~BeaconService();
};

#endif

