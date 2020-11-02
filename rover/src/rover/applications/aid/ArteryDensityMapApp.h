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
#include "rover/common/positionMap/IDensityMapHandler.h"
#include "rover/common/positionMap/RegularGridMap.h"
#include "rover/common/util/FileWriter.h"

using namespace omnetpp;
using namespace inet;

namespace rover {

class ArteryDensityMapApp
    : public AidBaseApp,
      public IDensityMapHandler<RegularGridMap<std::string>>,
      public omnetpp::cListener {
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
  using omnetpp::cIListener::finish;  // [-Woverloaded-virtual]
  virtual void finish() override;
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

  virtual void sendMapMap();
  virtual bool mergeReceivedMap(Packet *packet);
  //
  virtual void updateLocalMap() override;
  virtual void writeMap() override;
  virtual std::shared_ptr<Grid> getMap() override;

 private:
  // application
  artery::Middleware *middleware = nullptr;

  std::shared_ptr<OsgCoordinateConverter> converter;
  std::shared_ptr<Grid> dMap;
  std::unique_ptr<FileWriter> fileWriter;
  simtime_t lastUpdate = -1.0;

  std::string mapType;
  std::string mapTypeLog;
};

} /* namespace rover */
