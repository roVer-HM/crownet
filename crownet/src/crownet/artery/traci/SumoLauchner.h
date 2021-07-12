#pragma once

#include <traci/ConnectLauncher.h>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


using namespace traci;
using namespace omnetpp;


namespace crownet {

class SumoLauchner : public traci::ConnectLauncher {
 public:
  void initialize() override;
  virtual void initializeServer(std::shared_ptr<API> api) override;

};

} /* namespace crownet */
