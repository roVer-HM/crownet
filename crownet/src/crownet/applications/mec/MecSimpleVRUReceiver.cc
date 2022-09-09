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

#include "MecSimpleVRUReceiver.h"

namespace crownet {

Define_Module(MecSimpleVRUReceiver);

MecSimpleVRUReceiver::MecSimpleVRUReceiver() {
    // TODO Auto-generated constructor stub

}

MecSimpleVRUReceiver::~MecSimpleVRUReceiver() {
    // TODO Auto-generated destructor stub
}

void MecSimpleVRUReceiver::initialize(int stage)
{
    MecAppBase::initialize(stage);

    // avoid multiple initializations
    if (stage!=inet::INITSTAGE_APPLICATION_LAYER)
        return;

    //retrieving parameters
    size_ = par("packetSize");

    // set Udp Socket
    ueSocket.setOutputGate(gate("socketOut"));

    localUePort = par("localUePort");
    ueSocket.bind(localUePort);

    EV << "MecSimpleVRUReceiver Started!" << endl;

    // connect with the service registry
    cMessage *msg = new cMessage("connectMp1");
    scheduleAt(simTime() + 0, msg);

}

void MecSimpleVRUReceiver::handleMessage(cMessage *msg)
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
    MecAppBase::handleMessage(msg);

}

void MecSimpleVRUReceiver::finish(){
    MecAppBase::finish();
}

void MecSimpleVRUReceiver::handleUeMessage(omnetpp::cMessage *msg)
{
    EV << "MecSimpleVRUReceiver Message from UE" << endl;
}

void MecSimpleVRUReceiver::modifySubscription()
{
    // TODO??
}

void MecSimpleVRUReceiver::sendSubscription()
{
    // TODO??
}

void MecSimpleVRUReceiver::sendDeleteSubscription()
{
    //TODO??
}

void MecSimpleVRUReceiver::established(int connId)
{
    EV << "MecSimpleVRUReceiver Established" << endl;
}

void MecSimpleVRUReceiver::handleMp1Message()
{
    EV << "MecSimpleVRUReceiver MP1" << endl;
    //TODO
}

void MecSimpleVRUReceiver::handleServiceMessage()
{
    EV << "MecSimpleVRUReceiver Service Msg" << endl;
    //TODO??
}

void MecSimpleVRUReceiver::handleSelfMessage(cMessage *msg)
{
    //TODO
    delete msg;
}

}

