/*
 * ArteryNeighbourhood.h
 *
 *  Created on: Aug 21, 2020
 *      Author: sts
 */

#pragma once

#include <omnetpp.h>
#include <memory>
#include <vanetza/net/mac_address.hpp>
#include "artery/application/Middleware.h"
#include "artery/application/MovingNodeDataProvider.h"
#include "artery/networking/Router.h"
#include "inet/common/InitStages.h"
#include "rover/common/GridDensityMap.h"
#include "rover/common/OsgCoordConverter.h"

using namespace omnetpp;
using namespace inet;

namespace rover {

class ArteryNeighbourhood : public cSimpleModule {
 public:
  using ArteryGridDensityMap = GridDensityMap<std::string>;
  virtual ~ArteryNeighbourhood();

 protected:
  virtual int numInitStages() const override { return NUM_INIT_STAGES; }
  virtual void initialize(int stage) override;

  virtual void handleMessage(cMessage *message) override;

  virtual void updateLocalMap();
  virtual void sendLocalMap();
  virtual void receiveMapUpdate(cMessage *pkt);

 private:
  artery::Middleware *middleware;
  simtime_t updateInterval;
  OsgCoordConverter *converter_m;
  cMessage *localMapUpdate;
  cMessage *sendMap;
  std::shared_ptr<ArteryGridDensityMap> dMap;
  double gridSize;
  std::string id;
};

} /* namespace rover */
