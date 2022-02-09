/*
 * VadereLauncher.cc
 *
 *  Created on: Aug 6, 2020
 *      Author: sts
 */

#include "VadereLauncher.h"

#include "crownet/common/converter/OsgCoordConverter.h"
#include "inet/common/ModuleAccess.h"
#include "crownet/artery/traci/VadereApi.h"
#include "crownet/artery/traci/VadereUtils.h"
#include "crownet/crownet.h"


#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

using namespace traci;
using namespace omnetpp;

namespace crownet {



Define_Module(VadereLauncher);

void VadereLauncher::initialize()
{
    traci::ConnectLauncher::initialize();
    // check config override if simulation is run in parallel and container names must be set central
    auto hostPortOverride = getHostPortConfigOverride(CFGID_VADERE_HOST);
    if(hostPortOverride.first != ""){
        EV_INFO << "Config override found for hostname (--vadere-host=XXX:YYYY)! replace " << m_endpoint.hostname << "-->" <<
                hostPortOverride.first << std::endl;
        m_endpoint.hostname = hostPortOverride.first;

    }
    if(hostPortOverride.second != -1){
        EV_INFO << "Config override found for port (--vadere-host=XXX:YYYY)! replace " << m_endpoint.port << "-->" <<
                hostPortOverride.first << std::endl;
        m_endpoint.port = hostPortOverride.second;
    }
}

std::shared_ptr<API> VadereLauncher::createAPI() {
  return std::make_shared<VadereApi>();
}


void VadereLauncher::initializeServer(std::shared_ptr<API> api) {
  auto vApi = std::dynamic_pointer_cast<VadereApi>(api);

  // use current directory as fallback to create absolute paths for Vadere.
  fs::path iniBaseDir = fs::current_path();
  fs::path resultDir (cSimulation::getActiveSimulation()
                                                    ->getEnvir()
                                                    ->getConfigEx()
                                                    ->getVariable(CFGVAR_RESULTDIR));
  resultDir = resultDir.is_relative() ? fs::absolute(resultDir, iniBaseDir) : resultDir;

  fs::path vSPath (par("vadereScenarioPath").stdstringValue());
  vSPath = vSPath.is_relative() ? fs::absolute(vSPath, iniBaseDir) : vSPath;
  fs::path vCPath (par("vadereCachePath").stdstringValue());
  vCPath = vCPath.is_relative() ? fs::absolute(vCPath, iniBaseDir) : vCPath;

  vadere::VadereScenario scenario =
      vadere::getScenarioContent(vSPath.string());

  // get scenarioHash for cache location
  std::string scenarioHash =
          vApi->v_simulation.getScenarioHash(scenario.second);

  std::vector<vadere::VadereCache> cachePaths;
  cachePaths = vadere::getCachePaths(vCPath.string(), scenarioHash);

  vadere::SimCfg simCfg;
  simCfg.oppResultRootDir = resultDir.string();
  simCfg.oppConfigName = cSimulation::getActiveSimulation()
                             ->getEnvir()
                             ->getConfigEx()
                             ->getVariable(CFGVAR_CONFIGNAME);
  simCfg.oppExperiment = cSimulation::getActiveSimulation()
                             ->getEnvir()
                             ->getConfigEx()
                             ->getVariable(CFGVAR_EXPERIMENT);
  simCfg.oppDateTime = cSimulation::getActiveSimulation()
                           ->getEnvir()
                           ->getConfigEx()
                           ->getVariable(CFGVAR_DATETIME);



  simCfg.oppIterationVariables = cSimulation::getActiveSimulation()
                                     ->getEnvir()
                                     ->getConfigEx()
                                     ->getVariable(CFGVAR_ITERATIONVARSF);
  simCfg.oppRepetition = cSimulation::getActiveSimulation()
                             ->getEnvir()
                             ->getConfigEx()
                             ->getVariable(CFGVAR_REPETITION);
  cConfigOption* vecObj = cConfigOption::find("output-vector-file");
  if (vecObj) {
    simCfg.oppOutputVecFile = cSimulation::getActiveSimulation()
                                  ->getEnvir()
                                  ->getConfig()
                                  ->getAsFilename(vecObj);
  } else {
    simCfg.oppOutputVecFile = "";
  }
  cConfigOption* scaObj = cConfigOption::find("output-scalar-file");
  if (vecObj) {
    simCfg.oppOutputScalarFile = cSimulation::getActiveSimulation()
                                     ->getEnvir()
                                     ->getConfig()
                                     ->getAsFilename(scaObj);
  } else {
    simCfg.oppOutputScalarFile = "";
  }
  int seed = par("seed").intValue();
  if (seed == -1) {
    // default seed is current repetition
    const char* seed_s = cSimulation::getActiveSimulation()
                             ->getEnvir()
                             ->getConfigEx()
                             ->getVariable(CFGVAR_RUNNUMBER);
    seed = atoi(seed_s);
  }
  simCfg.seed = seed;
  simCfg.useVadereSeed = par("useVadereSeed").boolValue();

  vApi->v_simulation.sendSimulationConfig(simCfg);

  vApi->sendFile(scenario.first, scenario.second);
}

} /* namespace crownet */
