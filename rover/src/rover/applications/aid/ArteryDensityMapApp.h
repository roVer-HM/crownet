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
#include "rover/common/converter/OsgCoordConverter.h"
#include "rover/common/positionMap/RegularGridMap.h"
#include "rover/common/util/FileWriter.h"

using namespace omnetpp;
using namespace inet;

namespace rover {

class ArteryDensityMapApp : public AidBaseApp, public omnetpp::cListener {
 public:
  using Grid = RegularGridMap<std::string>;
  using Measurement =
      RegularGridMap<std::string>::node_mapped_type::element_type;
  using CellId = Grid::cell_key_type;

  virtual ~ArteryDensityMapApp();

 protected:
  // cSimpleModule
  virtual int numInitStages() const override { return NUM_INIT_STAGES; }
  virtual void initialize(int stage) override;
  // cListener

  void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj,
                     cObject *details) override;

  // Aid Socket
  virtual void setAppRequirements() override;
  virtual void setAppCapabilities() override;
  virtual void socketDataArrived(AidSocket *socket, Packet *packet) override;

  // FSM
  virtual void handleMessageWhenUp(cMessage *msg) override;
  virtual void setupTimers() override;
  virtual FsmState fsmAppMain(cMessage *msg) override;
  virtual FsmState fsmSetup(cMessage *msg) override;

  virtual bool mergeReceivedMap(Packet *packet);
  virtual void updateLocalMap();
  virtual void sendMapMap();

 private:
  // application
  artery::Middleware *middleware;
  std::shared_ptr<OsgCoordinateConverter> converter;

  std::shared_ptr<Grid> dMap;

  std::unique_ptr<FileWriter> fileWriter;
};

} /* namespace rover */
