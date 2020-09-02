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
#include "rover/applications/common/AidBaseApp.h"
#include "rover/common/GridDensityMap.h"
#include "rover/common/OsgCoordConverter.h"

using namespace omnetpp;
using namespace inet;

namespace rover {

class ArteryNeighbourhood : public AidBaseApp {
 public:
  using ArteryGridDensityMap = GridDensityMap<std::string>;
  virtual ~ArteryNeighbourhood();

 protected:
  virtual int numInitStages() const override { return NUM_INIT_STAGES; }
  virtual void initialize(int stage) override;
  virtual void setAppRequirements() override;
  virtual void setAppCapabilities() override;

  virtual void handleMessageWhenUp(cMessage *msg) override;
  virtual void setupTimers() override;
  virtual FsmState fsmAppMain(cMessage *msg) override;
  virtual void socketDataArrived(AidSocket *socket, Packet *packet) override;

  virtual void updateLocalMap();
  virtual void sendLocalMap();

 private:
  // timer
  cMessage *localMapUpdate;
  cMessage *sendMap;
  simtime_t updateInterval;

  // aid socket

  // application
  artery::Middleware *middleware;
  OsgCoordConverter *converter_m;
  std::shared_ptr<ArteryGridDensityMap> dMap;
  double gridSize;
  std::string id;
};

} /* namespace rover */
