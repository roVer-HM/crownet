
#include "crownet/artery/traci/VariableCache.h"

namespace crownet {

VaderePersonCache::VaderePersonCache(traci::LiteAPI& api, const std::string& id)
    : traci::VariableCache(api, CMD_GET_PERSON_VARIABLE, id) {}

VadereSimulationCache::VadereSimulationCache(traci::LiteAPI& api)
    : traci::VariableCache(api, CMD_GET_SIM_VARIABLE, "") {}
}  // namespace crownet
