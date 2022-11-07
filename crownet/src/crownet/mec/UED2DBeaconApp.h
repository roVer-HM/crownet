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

#include <omnetpp.h>
#include <common/binder/Binder.h>
#include "inet/mobility/contract/IMobility.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

#ifndef APPS_MEC_MECBEACON_UED2DBEACONAPP_H_
#define APPS_MEC_MECBEACON_UED2DBEACONAPP_H_

using namespace omnetpp;

class UED2DBeaconApp : public cSimpleModule {
private:
    inet::UdpSocket socket;
    simtime_t period_;
    int localPort;

    inet::IMobility *mobility;
    inet::L3Address destAddress_;
    int ueId;

    cOutVector beaconArrivalVector;

public:
    UED2DBeaconApp();
    virtual ~UED2DBeaconApp();

protected:
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void initialize(int) override;
    virtual void handleMessage(cMessage *msg) override;

private:
    void sendBeacon();
};

#endif /* APPS_MEC_MECBEACON_UED2DBEACONAPP_H_ */
