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


#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <inet/common/ModuleAccess.h>
#include "inet/common/packet/Message.h"
#include "crownet/artery/traci/TraCiForwarder.h"
#include "crownet/artery/traci/VadereCore.h"
#include "crownet/crownet.h"
#include "crownet/applications/control/control_m.h"
#include "crownet/dcd/identifier/CellKeyProvider.h"

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
        api->setControlHandler(this);
        controlGate = par("controlGate").stdstringValue();

    }
}

void ControlManager::handleMessage(cMessage *msg)
{
    throw cRuntimeError("Module does not handle messages");
}

void ControlManager::handleActionCommand(const ControlCmd& cmd){
    Enter_Method_Silent();
    cModule* sendingApp = this->findModuleByPath(cmd.sendingNode.c_str());
    if (!sendingApp){
        throw cRuntimeError("Cannot find module with path %s", cmd.sendingNode.c_str());
    }

    Message* msg = new Message();
    auto data = msg->addTagIfAbsent<SimpleControlCfg>();
    data->setModelString(cmd.model.c_str());
    data->setModelData(cmd.message.c_str());

    try {
        std::stringstream ss;
        ss << cmd.message;

        boost::property_tree::ptree pt;
        boost::property_tree::read_json(ss, pt);

        //TODO: discuss what to do with the time attribute
        //data->setAppTTL(pt.get<double>("time"));


    }
    catch (std::exception const& e)
    {
        throw cRuntimeError("Cannot parse message from controller into property_tree (Json): %s", e.what());
    }
    this->sendDirect(msg, sendingApp, controlGate.c_str());

}

std::vector<double> ControlManager::handleDensityMapCommand(const DensityMapCmd& cmd){
    Enter_Method_Silent();


    auto node_module = this->findModuleByPath(cmd.nodeId.c_str());
    IDensityMapHandlerBase<RegularDcdMap>* map_handler =
            check_and_cast<IDensityMapHandlerBase<RegularDcdMap>*>(node_module);

    std::vector<double> density_vals;
    auto map = map_handler->getMap();
    auto cellProvider = std::dynamic_pointer_cast<GridCellIDKeyProvider>(map->getCellKeyProvider());

    density_vals.push_back(cellProvider->getGridSize().x);
    density_vals.push_back(cellProvider->getGridSize().y);
    density_vals.push_back(cellProvider->getCellSize().x);
    density_vals.push_back(cellProvider->getCellSize().y);
    auto iter = map->valid();
     for( auto item: iter){
         const auto measure = item.second.val();
         if(measure){
             auto x = item.first.val().first;
             auto y = item.first.val().second;
             auto count = item.second.val()->getCount();
             density_vals.push_back(x);
             density_vals.push_back(y);
             density_vals.push_back(count);
             EV_INFO << "x-y-count: " <<
                     x << "-" << y <<"-"<< count << std::endl;
         }
    }

     return density_vals;
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

    // check config override if simulation is run in parallel and container names must be set central
    auto hostPortOverride = getHostPortConfigOverride(CFGID_FLOW_HOST);
    if(hostPortOverride.first != ""){
        EV_INFO << "Config override found for hostname (--flow-host=XXX:YYYY)! replace " << host<< "-->" <<
                hostPortOverride.first << std::endl;
        host = hostPortOverride.first;

    }
    if(hostPortOverride.second != -1){
        EV_INFO << "Config override found for port (--flow-host=XXX:YYYY)! replace " << port << "-->" <<
                hostPortOverride.first << std::endl;
        port = hostPortOverride.second;
    }

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
