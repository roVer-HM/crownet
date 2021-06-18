/*
 * VadereLauchner.cc
 *
 *  Created on: Aug 6, 2020
 *      Author: sts
 */

#include "crownet/artery/traci/SumoLauchner.h"
#include "crownet/crownet.h"

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

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

void SumoLauchner::initializeServer(std::shared_ptr<API> api){

    int seed = par("seed").intValue();
    if (seed == -1) {
        seed = uniform(0, std::numeric_limits<int>::max()); //
    }
    std::stringstream ss;
    ss << seed;

    // use current directory as fallback to create absolute paths for Vadere.
    fs::path iniBaseDir = fs::current_path();
    fs::path cfgFilePath (par("sumoCfgBase").stdstringValue());
    cfgFilePath = cfgFilePath.is_relative() ? fs::absolute(cfgFilePath, iniBaseDir) : cfgFilePath;

    // add seed child
    cXMLElement* cfgFile = getEnvir()->getXMLDocument(cfgFilePath.c_str());
    cXMLElement* seed_node = new cXMLElement("seed", cfgFile);
    seed_node->setAttribute("value", ss.str().c_str());
    cfgFile->appendChild(seed_node);

    // add basedir child
    cXMLElement* basedir_node = new cXMLElement("basedir", cfgFile);
    basedir_node->setAttribute("path", cfgFilePath.parent_path().c_str());
    cfgFile->appendChild(basedir_node);

    std::string xmlString = cfgFile->getXML();

    // cfgFilePath.filename().string()
    // fixme: veins dispatcher script expects this name.
    api->sendFile("sumo-launchd.launch.xml", xmlString);
}

} /* namespace crownet */
