#pragma once

#include <traci/ConnectLauncher.h>


using namespace traci;

namespace crownet {

class SumoLauchner : public traci::ConnectLauncher {
 public:
  void initialize() override;
  virtual void initializeServer(std::shared_ptr<API> api) override;
};

} /* namespace crownet */
