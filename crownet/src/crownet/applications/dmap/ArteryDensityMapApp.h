/*
 * ArteryNeighbourhood.h
 *
 *  Created on: Aug 21, 2020
 *      Author: sts
 */

#pragma once

#include <omnetpp.h>
#include <omnetpp/cwatch.h>
#include <memory>
#include <vanetza/net/mac_address.hpp>
#include "artery/application/Middleware.h"
#include "artery/application/MovingNodeDataProvider.h"
#include "artery/networking/Router.h"
#include "artery/utility/IdentityRegistry.h"
#include "crownet/crownet.h"

#include "inet/common/InitStages.h"
#include "crownet/applications/dmap/BaseDensityMapApp.h"
#include "crownet/applications/common/BaseApp.h"
#include "crownet/common/IDensityMapHandler.h"
#include "crownet/common/converter/OsgCoordConverter.h"
#include "crownet/common/util/FileWriter.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"

using namespace omnetpp;
using namespace inet;

namespace crownet {


class ArteryDensityMapApp : public BaseDensityMapApp {
 public:
  virtual ~ArteryDensityMapApp() = default;
  ArteryDensityMapApp(){};

 protected:
  // cSimpleModule
  virtual void initialize(int stage) override;

  // cListener
  void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj,
                     cObject *details) override;

  // FSM
  virtual FsmState fsmSetup(cMessage *msg) override;

  // IDensityMapHandler
  virtual void updateLocalMap() override;

 private:
  // application
  artery::Middleware *middleware = nullptr;
  artery::IdentityRegistry *identiyRegistry = nullptr;
};

} /* namespace crownet */
