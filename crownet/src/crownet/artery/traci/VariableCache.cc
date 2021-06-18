
#include "crownet/artery/traci/VariableCache.h"

namespace crownet {

VaderePersonCache::VaderePersonCache(std::shared_ptr<API> api, const std::string& id)
    : traci::VariableCache(std::dynamic_pointer_cast<VadereApi>(api), libsumo::CMD_GET_PERSON_VARIABLE, id) {}




VadereSimulationCache::VadereSimulationCache(std::shared_ptr<API> api)
    : traci::VariableCache(std::dynamic_pointer_cast<VadereApi>(api), libsumo::CMD_GET_SIM_VARIABLE, "") {}
}  // namespace crownet
