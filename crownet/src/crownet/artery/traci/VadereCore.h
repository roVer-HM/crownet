/*
 * RoVerCore.h
 *
 *  Created on: Aug 6, 2020
 *      Author: sts
 */

#pragma once

#include <traci/Core.h>
#include "crownet/artery/traci/VadereLiteApi.h"

using namespace traci;

namespace crownet {

class VadereCore : public traci::Core {
 public:
  virtual VadereLiteApi* getVadereLiteAPI();

 protected:
  virtual void handleMessage(omnetpp::cMessage*) override;
};

} /* namespace crownet */
