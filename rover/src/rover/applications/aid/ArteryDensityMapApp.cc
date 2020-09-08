/*
 * ArteryNeighbourhood.cc
 *
 *  Created on: Aug 21, 2020
 *      Author: sts
 */

#include "rover/applications/aid/ArteryDensityMapApp.h"

#include <inet/common/ModuleAccess.h>
#include <vanetza/geonet/areas.hpp>
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
    gridSize = par("gridSize").doubleValue();
  } else if (stage == INITSTAGE_TRANSPORT_LAYER) {
    middleware = inet::findModuleFromPar<artery::Middleware>(
        par("middelwareModule"), this, true);
    converter_m = inet::getModuleFromPar<OsgCoordConverter>(
        par("coordConverterModule"), this, true);
    std::ostringstream node_id;

    node_id << middleware->getFacilities().getConst<artery::Identity>().traci
            << ":"
            << middleware->getFacilities()
                   .getConst<artery::Identity>()
                   .host->getIndex()
            << ":"
            << middleware->getFacilities()
                   .getConst<artery::Identity>()
                   .host->getId();
    dMap = std::make_shared<RegularGridMap>(node_id.str(), gridSize);

    if (par("writeDensityLog").boolValue()) {
      FileWriterBuilder fBuilder{};
      fBuilder.addMetadata("XSIZE",
                           converter_m->getConverter()->getBoundaryWidth());
      fBuilder.addMetadata("YSIZE",
                           converter_m->getConverter()->getBoundaryHeight());
      fBuilder.addMetadata("CELLSIZE", gridSize);

      fileWriter.reset(fBuilder.build());
      fileWriter->writeHeader(
          {"simtime", "x", "y", "count", "measure_t", "received_t"});
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

void ArteryDensityMapApp::handleMessageWhenUp(cMessage *msg) {
  AidBaseApp::handleMessageWhenUp(msg);
}

BaseApp::FsmState ArteryDensityMapApp::fsmAppMain(cMessage *msg) {
  // send Message
  updateLocalMap();
  sendLocalMap();
  scheduleNextAppMainEvent();
  return FsmRootStates::WAIT_ACTIVE;
}

void ArteryDensityMapApp::socketDataArrived(AidSocket *socket, Packet *packet) {
  auto p = checkEmitGetReceived<PositionMapPacket>(packet);

  int numCells = p->getNumCells();
  std::string _nodeId = p->getNodeId();
  simtime_t _received = simTime();
  for (int i = 0; i < numCells; i++) {
    CellId _cId = std::make_pair(p->getCellX(i), p->getCellY(i));
    simtime_t _measured = p->getMTime(i);
    auto _m = std::make_shared<DensityMeasure>(p->getCellCount(i), _measured,
                                               _received);
    dMap->update(_cId, _nodeId, _m);
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
  const auto &posInet = converter_m->getConverter()->position_cast_inet(pos);

  const auto &table =
      middleware->getFacilities().getConst<artery::Router>().getLocationTable();

  // count yourself
  dMap->incrementLocal(posInet, measureTime, true);

  vanetza::geonet::LocationTable::entry_visitor eVisitor =
      [this, &measureTime](const vanetza::MacAddress &mac,
                           const vanetza::geonet::LocationTableEntry &entry) {
        if (!entry.has_position_vector()) return;
        const vanetza::geonet::GeodeticPosition geoPos =
            entry.get_position_vector().position();
        auto cartPos =
            converter_m->getConverter()->convertToCartInetPosition(geoPos);

        using namespace omnetpp;
        EV_DEBUG << "process: " << mac << " geo:[" << geoPos.latitude.value()
                 << "|" << geoPos.longitude.value() << "]\n";
        dMap->incrementLocal(cartPos, measureTime);
      };
  table.visit(eVisitor);

  using namespace omnetpp;
  EV_DEBUG << "[ " << dMap->getNodeIde() << "] ";
  EV_DEBUG << dMap->getView("ymf")->str();
}

void ArteryDensityMapApp::sendLocalMap() {
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
