#pragma once

#include <traci/ConnectLauncher.h>


using namespace traci;

namespace crownet {

class SumoLauchner : public traci::ConnectLauncher {
 public:
  void initialize() override;
};

} /* namespace crownet */
