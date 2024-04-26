
#pragma once

#include <traci/VariableCache.h>
#include <string>
#include "crownet/artery/traci/VadereApi.h"

namespace crownet {

class VaderePersonCache : public traci::PersonCache {
 public:
  VaderePersonCache(std::shared_ptr<API> api, const std::string& id);
};

class VadereSimulationCache : public traci::VariableCache {
 public:
  VadereSimulationCache(std::shared_ptr<API> api);
};

}  // namespace crownet
