/*
 * VadereLauncher.h
 *
 *  Created on: Aug 6, 2020
 *      Author: sts
 */

#pragma once

#include <omnetpp.h>
#include <omnetpp/csimplemodule.h>
#include <traci/ConnectLauncher.h>
#include "crownet/artery/traci/VadereApi.h"

using namespace traci;

namespace crownet {

class VadereLauncher : public traci::ConnectLauncher {
 public:
  void initialize() override;
  virtual std::shared_ptr<API> createAPI() override;
  virtual void initializeServer(std::shared_ptr<API> api) override;
};

} /* namespace crownet */
