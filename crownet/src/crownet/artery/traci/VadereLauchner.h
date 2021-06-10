/*
 * VadereLauchner.h
 *
 *  Created on: Aug 6, 2020
 *      Author: sts
 */

#pragma once

#include <omnetpp.h>
#include <omnetpp/csimplemodule.h>
#include <traci/ConnectLauncher.h>
#include "crownet/artery/traci/VadereLiteApi.h"

using namespace traci;

namespace crownet {

class VadereLauchner : public traci::ConnectLauncher {
 public:
  void initialize() override;
  std::pair<API*, LiteAPI*> createAPI() override;
  void initializeServer(LiteAPI* m_lite) override;
};

} /* namespace crownet */
