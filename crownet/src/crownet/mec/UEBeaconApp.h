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


#ifndef __SIMU5G_UEBEACONAPP_H_
#define __SIMU5G_UEBEACONAPP_H_

#include <omnetpp.h>
#include "common/binder/Binder.h"
#include "inet/mobility/contract/IMobility.h"

#include "packets/SimpleMECBeacon_m.h"
#include <apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_m.h>
#include <apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_Types.h>

#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

#include "inet/common/TimeTag_m.h"

using namespace omnetpp;


class UEBeaconApp : public cSimpleModule
{
  private:
    inet::UdpSocket socket;

    inet::IMobility *mobility;
    std::string mecAppName;

    simtime_t period_;

    int ueId;
    int beaconType;

    int localPort_;
    int deviceAppPort_;

    char* sourceSimbolicAddress;
    char* deviceSimbolicAppAddress_;
    inet::L3Address deviceAppAddress_;

    inet::L3Address mecAppAddress_;
    int mecAppPort_;

    cMessage *selfStart_;
    cMessage *selfStop_;
    cMessage *selfBeacon_;

    cOutVector beaconArrivalVector;

  protected:
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }

    virtual void initialize(int) override;
    virtual void handleMessage(cMessage *msg) override;

  private:
    void sendStart();
    void startAck(cMessage*);
    void sendBeacon();
    void handleBeacons(const ResponseBeaconMessage*);

};

#endif
