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

#ifndef CROWNET_APPLICATIONS_MEC_MECSIMPLEVRURECEIVER_H_
#define CROWNET_APPLICATIONS_MEC_MECSIMPLEVRURECEIVER_H_

#include "apps/mec/MecApps/MecAppBase.h"

namespace crownet {

class MecSimpleVRUReceiver : public MecAppBase {
protected:
    //UDP socket to communicate with the UeApp
   inet::UdpSocket ueSocket;
   int localUePort;

   inet::L3Address ueAppAddress;
   int ueAppPort;


   int size_;
   std::string subId;

    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void finish() override;

    virtual void handleServiceMessage() override;
    virtual void handleMp1Message() override;

    virtual void handleUeMessage(omnetpp::cMessage *msg) override;

    virtual void modifySubscription();
    virtual void sendSubscription();
    virtual void sendDeleteSubscription();

    virtual void handleSelfMessage(cMessage *msg) override;

    virtual void established(int connId) override;

public:
    MecSimpleVRUReceiver();
    virtual ~MecSimpleVRUReceiver();
};

}

#endif /* CROWNET_APPLICATIONS_MEC_MECSIMPLEVRURECEIVER_H_ */
