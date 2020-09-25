/*
 * ArteryNeighbourhood.cc
 *
 *  Created on: Aug 21, 2020
 *      Author: sts
 */

#include "rover/applications/aid/ArteryDensityMapApp.h"

#include <inet/common/ModuleAccess.h>
#include <memory>
#include <vanetza/geonet/areas.hpp>
#include "artery/utility/IdentityRegistry.h"
#include "rover/applications/PositionMapPacket_m.h"
#include "traci/Core.h"
#include "traci/LiteAPI.h"
#include "traci/Position.h"

namespace rover {

Define_Module(ArteryDensityMapApp);

ArteryDensityMapApp::~ArteryDensityMapApp() {}

void ArteryDensityMapApp::initialize(int stage) {
  AidBaseApp::initialize(stage);
  if (stage == INITSTAGE_LOCAL) {
    std::string x = par("coordConverterModule").stdstringValue();
  } else if (stage == INITSTAGE_APPLICATION_LAYER) {
    middleware = inet::findModuleFromPar<artery::Middleware>(
        par("middelwareModule"), this, true);
    converter = inet::getModuleFromPar<OsgCoordConverter>(
                    par("coordConverterModule"), this, true)
                    ->getConverter();

    // subscribe updateSignal at host module level (pedestrian, vehicle) to
    // catch identity changes.
    auto hostModule =
        middleware->getFacilities().getConst<artery::Identity>().host;
    hostModule->subscribe(artery::IdentityRegistry::updateSignal, this);
  }
}

void ArteryDensityMapApp::receiveSignal(cComponent *source,
                                        simsignal_t signalID, cObject *obj,
                                        cObject *details) {
  if (signalID == artery::IdentityRegistry::updateSignal) {
    std::ostringstream node_id;
    node_id << middleware->getFacilities()
                   .getConst<artery::Identity>()
                   .geonet.mid();

    std::pair<int, int> gridDim;
    double gridSize = par("gridSize").doubleValue();
    gridDim.first = floor(converter->getBoundaryWidth() / gridSize);
    gridDim.second = floor(converter->getBoundaryWidth() / gridSize);
    dMap = std::make_shared<Grid>(node_id.str(), gridSize, gridDim);

    if (par("writeDensityLog").boolValue()) {
      FileWriterBuilder fBuilder{};
      fBuilder.addMetadata("IDXCOL", 3);
      fBuilder.addMetadata("XSIZE", converter->getBoundaryWidth());
      fBuilder.addMetadata("YSIZE", converter->getBoundaryHeight());
      fBuilder.addMetadata("CELLSIZE", par("gridSize").doubleValue());
      fBuilder.addPath(node_id.str());

      fileWriter.reset(fBuilder.build());
      fileWriter->writeHeader(
          {"simtime", "x", "y", "count", "measured_t", "received_t"});
    }
  }
}

void ArteryDensityMapApp::setAppRequirements() {
  L3Address destAddr = chooseDestAddr();
  socket.setAppRequirements(1.0, 2.0, AidRecipientClass::ALL_LOCAL, destAddr,
                            destPort);
}

void ArteryDensityMapApp::setAppCapabilities() {
  // todo: no CAP right now.
}

void ArteryDensityMapApp::setupTimers() {
  if (destAddresses.empty()) {
    cRuntimeError("No address set.");
  } else {
    // schedule at startTime or current time, whatever is bigger.
    scheduleNextAppMainEvent(std::max(startTime, simTime()));
  }
}

BaseApp::FsmState ArteryDensityMapApp::fsmSetup(cMessage *msg) {
  // ensure Density Grid map was initialized by event
  if (dMap == nullptr)
    throw omnetpp::cRuntimeError(
        "Density Grid map not initialized. Was the "
        "artery::IdentityRegistry::updateSignal event fired? ");
  return AidBaseApp::fsmSetup(msg);
}

void ArteryDensityMapApp::handleMessageWhenUp(cMessage *msg) {
  AidBaseApp::handleMessageWhenUp(msg);
}

BaseApp::FsmState ArteryDensityMapApp::fsmAppMain(cMessage *msg) {
  // send Message
  updateLocalMap();
  sendMapMap();
  scheduleNextAppMainEvent();
  return FsmRootStates::WAIT_ACTIVE;
}

void ArteryDensityMapApp::socketDataArrived(AidSocket *socket, Packet *packet) {
  auto p = checkEmitGetReceived<PositionMapPacket>(packet);

  int numCells = p->getNumCells();
  std::string _nodeId = p->getNodeId();
  simtime_t _received = simTime();
  for (int i = 0; i < numCells; i++) {
    CellId _cId{p->getCellX(i), p->getCellY(i)};
    simtime_t _measured = p->getMTime(i);
    auto _m =
        std::make_shared<Measurement>(p->getCellCount(i), _measured, _received);
    dMap->update(_cId, _nodeId, std::move(_m));
  }

  using namespace omnetpp;
  EV_DEBUG << dMap->getView("ymf")->str();
  EV_DEBUG << dMap->getView("local")->str();

  delete packet;
  socketFsmResult =
      FsmRootStates::WAIT_ACTIVE;  // GoTo WAIT_ACTIVE steady state
}

void ArteryDensityMapApp::updateLocalMap() {
  dMap->resetLocalMap();  // clear local map
  simtime_t measureTime = simTime();

  // add yourself to the map.
  const auto &pos = middleware->getFacilities()
                        .getConst<artery::MovingNodeDataProvider>()
                        .position();
  const auto &posInet = converter->position_cast_traci(pos);
  dMap->incrementLocalOwnPos(posInet, measureTime);

  // visitor for artery location table
  vanetza::geonet::LocationTable::entry_visitor eVisitor =
      [this, &measureTime](const vanetza::MacAddress &mac,
                           const vanetza::geonet::LocationTableEntry &entry) {
        // only entries with valid position vector
        if (!entry.has_position_vector()) return;

        // convert Geo to 2D-Cartesian
        const vanetza::geonet::GeodeticPosition geoPos =
            entry.get_position_vector().position();
        auto cartPos = converter->convertToCartTraCIPosition(geoPos);

        // get string representation of  mac as id
        std::ostringstream _id;
        _id << mac;

        // increment density map
        dMap->incrementLocal(cartPos, _id.str(), measureTime);
      };

  // visit
  const auto &table =
      middleware->getFacilities().getConst<artery::Router>().getLocationTable();
  table.visit(eVisitor);

  using namespace omnetpp;
  EV_DEBUG << "[ " << dMap->getNodeId() << "] ";
  EV_DEBUG << dMap->getView("ymf")->str();
}

void ArteryDensityMapApp::sendMapMap() {
  const auto &payload = createPacket<PositionMapPacket>();
  auto ymfView = dMap->getView("ymf");
  int numValidCells = ymfView->size();
  simtime_t time = simTime();
  payload->setNumCells(numValidCells);
  payload->setNodeId(ymfView->getId().c_str());
  payload->setCellCountArraySize(numValidCells);
  payload->setCellXArraySize(numValidCells);
  payload->setCellYArraySize(numValidCells);
  payload->setMTimeArraySize(numValidCells);
  int currCell = 0;

  for (const auto &e : ymfView->range()) {
    const auto &cell = e.first;
    const auto &measure = e.second;
    payload->setCellX(currCell, cell.first);
    payload->setCellY(currCell, cell.second);
    payload->setCellCount(currCell, measure->getCount());
    payload->setMTime(currCell, measure->getMeasureTime());
    currCell++;

    if (par("writeDensityLog").boolValue()) {
      std::string del = fileWriter->del();
      fileWriter->write() << time.dbl() << del << cell.first << del
                          << cell.second << del << measure->csv(del)
                          << std::endl;
    }
  }

  payload->setChunkLength(B(1000));  // todo calc: 24 * currCell Fragmentation!
  sendPayload(payload);
}

} /* namespace rover */
