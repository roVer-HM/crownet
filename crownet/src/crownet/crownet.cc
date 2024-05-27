/*
 * crownet.cc
 *
 *  Created on: Nov 17, 2019
 *      Author: stsc
 */

#include <iostream>
#include "crownet/crownet.h"

namespace crownet {

Register_GlobalConfigOption(CFGID_VADERE_HOST, "vadere-host", CFG_STRING, "", "Host:Port(optional) override for vadere TraCI connection.");
Register_GlobalConfigOption(CFGID_SUMO_HOST, "sumo-host", CFG_STRING, "", "Host:Port(optional) override for sumo TraCI connection.");
Register_GlobalConfigOption(CFGID_FLOW_HOST, "flow-host", CFG_STRING, "", "Host:Port(optional) override for flowcontrol TraCI connection.");


std::pair<std::string, int> getHostPortConfigOverride(omnetpp::cConfigOption *entry){
    std::string host = omnetpp::cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getAsString(entry, "");
    if(host == ""){
        return std::make_pair("", -1);
    } else if (host.find(":") != std::string::npos){
        int n = host.find(":");
        int port = -1;
        try{
            port = std::stoi(host.substr(n+1, host.size()));
        } catch(...){
            throw omnetpp::cRuntimeError("Cannot parse HostPortConfigOverride %s", host.c_str());
        }
        host = host.substr(0, n);
        return std::make_pair(host, port);
    } else {
        return std::make_pair(host, -1);
    }
}

uint32_t simtime_to_timestamp_32_ms(simtime_t t){
    if (t < simtime_t::ZERO){
        t = simTime();
    }
    return (uint32_t)(t.inUnit(SimTimeUnit::SIMTIME_MS) & ((uint64_t)(1) << 32)-1);
}

simtime_t timestamp_32_ms_to_simtime(uint32_t tstamp, simtime_t base){
    if (base < 0){
        base = simTime(); // use current time
    }
    uint64_t t = (uint64_t)tstamp + (base.inUnit(SimTimeUnit::SIMTIME_MS) & 0xFFFFFFFF00000000);
    return SimTime(t, SimTimeUnit::SIMTIME_MS);
}

std::string dpmmMapTypeToString(const DpmmMapType& type){
    return std::string(dpmmMapTpyeString[(int)type]);
}


}
