/*
 * VadereLiteApi.h
 *
 *  Created on: Aug 6, 2020
 *      Author: sts
 */

#pragma once

#include <traci/LiteAPI.h>
#include "rover/artery/traci/VadereApi.h"

namespace rover {

class VadereLiteApi : public traci::LiteAPI {
 public:
  VadereLiteApi(VadereApi& api) : LiteAPI(api), v_api(api) {}

  VadereApi::VaderSimulationScope& vSimulation() { return v_api.v_simulation; }
  const VadereApi::VaderSimulationScope& vSimulation() const {
    return v_api.v_simulation;
  }
  VadereApi::VaderePersonScope& vPerson() { return v_api.v_person; }
  const VadereApi::VaderePersonScope& vPerson() const { return v_api.v_person; }

 private:
  VadereApi& v_api;
};

} /* namespace rover */
