/*
 * VadereLauncher.cc
 *
 *  Created on: Aug 6, 2020
 *      Author: sts
 */

#include "SumoLauncher.h"

#include "crownet/crownet.h"

namespace fs = boost::filesystem;

using namespace traci;
using namespace omnetpp;

namespace crownet {


Define_Module(SumoLauncher);

void SumoLauncher::initialize()
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

void SumoLauncher::initializeServer(std::shared_ptr<API> api){
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
    bool deleteXML = false;

    cXMLElement* cfgFile = getEnvir()->getXMLDocument(cfgFilePath.c_str());
    if (std::strcmp("launch", cfgFile->getName()) == 0){
        // launch.xml provided. Just use it.
    } else if (std::strcmp("configuration", cfgFile->getName()) == 0){
        // configuration file provided. Build launch xml
        cXMLElement* lauchnXml = new cXMLElement("launch", nullptr);

        auto inputs = cfgFile->getChildrenByTagName("input")[0];
        for(cXMLElement* input : inputs->getChildren()){
            cXMLElement* file = new cXMLElement("copy", lauchnXml);
            file->setAttribute("file", input->getAttribute("value"));
        }

        cXMLElement* file = new cXMLElement("copy", lauchnXml);
        file->setAttribute("file", cfgFilePath.filename().c_str());
        file->setAttribute("type", "config");
        cfgFile = lauchnXml;
        deleteXML = true;
    } else {
        throw cRuntimeError("expected launch or configuration root element got %s", cfgFile->getName());
    }

    cXMLElement* seed_node = new cXMLElement("seed", cfgFile);
    seed_node->setAttribute("value", ss.str().c_str());
    cfgFile->appendChild(seed_node);


    cXMLElement* basedir_node = new cXMLElement("basedir", cfgFile);
    basedir_node->setAttribute("path", cfgFilePath.parent_path().c_str());
    cfgFile->appendChild(basedir_node);


    std::string xmlString = cfgFile->getXML();

    // cfgFilePath.filename().string()
    // fixme: veins dispatcher script expects this name.
    api->sendFile("sumo-launchd.launch.xml", xmlString);
    if (deleteXML)
        delete cfgFile; // in case we created a new xml file manually delete it here
}


} /* namespace crownet */
