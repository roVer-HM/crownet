/*
 * RoVerCore.h
 *
 *  Created on: Aug 6, 2020
 *      Author: sts
 */

#pragma once

#include <traci/Core.h>
#include "crownet/artery/traci/VadereApi.h"
#include "crownet/artery/traci/TraCiForwarder.h"

using namespace traci;

namespace crownet {

class VadereCore : public traci::Core {
 public:
  virtual std::shared_ptr<TraCiForwarder> getTraCiForwarder();
  virtual std::shared_ptr<VadereApi> getVadereApi();

 protected:
  virtual void handleMessage(omnetpp::cMessage*) override;
};

} /* namespace crownet */
