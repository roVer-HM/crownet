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

#include "ControlManager.h"


#include <inet/common/ModuleAccess.h>
#include "crownet/artery/traci/TraCiForwarder.h"
#include "crownet/artery/traci/VadereCore.h"

namespace crownet {


Define_Module(ControlManager);

ControlManager::~ControlManager(){

}


void ControlManager::initialize(int stage)
{
    if (stage == INITSTAGE_APPLICATION_LAYER){
        // register TraCiForwardProvider in controller api and set controller as listener

        VadereCore* core =
            inet::getModuleFromPar<VadereCore>(par("coreModule"), this);
        subscribeTraCI(core);

        auto traciFw = core->getTraCiForwarder();
        api = std::make_shared<ControlTraCiApi>();
        api->setTraCiForwarder(traciFw);

    }
}

void ControlManager::handleMessage(cMessage *msg)
{
    throw cRuntimeError("Module does not handle messages");
}

void ControlManager::finish() {
  unsubscribeTraCI();
  cSimpleModule::finish();
}


void ControlManager::traciInit()
{
    Enter_Method_Silent();
    std::string host = par("host").stdstringValue();
    int port = par("port").intValue();
    try{
        api->connect(host, port);
    } catch(tcpip::SocketException e){
        throw omnetpp::cRuntimeError("Cannot connect to FlowControl server using <<%s:%s>>. "
                "Is server or container started or check hostname and port configuration?",
                host.c_str(), std::to_string(port).c_str());
    }
    nextTime = api->handleInit(simTime().dbl());
}

void ControlManager::traciStep()
{
    Enter_Method_Silent();
    // only call controller if nextTime is reached. (negative or zero: each event)
    if (nextTime <= simtime_t::ZERO || simTime() >= nextTime ){
        nextTime = api->handleSimStep(simTime().dbl());
    }
}

void ControlManager::traciClose()
{
    Enter_Method_Silent();
    // todo send close command
}


} // namespace
