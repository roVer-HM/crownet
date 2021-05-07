/*
 * VadereLauchner.cc
 *
 *  Created on: Aug 6, 2020
 *      Author: sts
 */

#include "crownet/artery/traci/SumoLauchner.h"
#include "crownet/crownet.h"

using namespace traci;
using namespace omnetpp;

namespace crownet {


Define_Module(SumoLauchner);

void SumoLauchner::initialize()
{
    traci::ConnectLauncher::initialize();
    // check config override if simulation is run in parallel and container names must be set central
    auto hostPortOverride = getHostPortConfigOverride(CFGID_SUMO_HOST);
    if(hostPortOverride.first != ""){
        EV_INFO << "Config override found for hostname (--sumo-host=XXX:YYYY)! replace " << m_endpoint.hostname << "-->" <<
                hostPortOverride.first << std::endl;
        m_endpoint.hostname = hostPortOverride.first;

    }
    if(hostPortOverride.second != -1){
        EV_INFO << "Config override found for port (--sumo-host=XXX:YYYY)! replace " << m_endpoint.port << "-->" <<
                hostPortOverride.first << std::endl;
        m_endpoint.port = hostPortOverride.second;
    }
}

} /* namespace crownet */
