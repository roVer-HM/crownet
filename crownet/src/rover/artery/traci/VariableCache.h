
#pragma once

#include <traci/VariableCache.h>
#include <string>

namespace rover {

class VaderePersonCache : public traci::VariableCache {
 public:
  VaderePersonCache(traci::LiteAPI&, const std::string& id);
};

class VadereSimulationCache : public traci::VariableCache {
 public:
  VadereSimulationCache(traci::LiteAPI&);
};

}  // namespace rover
