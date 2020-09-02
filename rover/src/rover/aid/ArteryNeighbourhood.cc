/*
 * ArteryNeighbourhood.cc
 *
 *  Created on: Aug 21, 2020
 *      Author: sts
 */

#include "ArteryNeighbourhood.h"
#include <inet/common/ModuleAccess.h>
#include <vanetza/geonet/areas.hpp>
#include "rover/aid/PositionMap_m.h"
#include "traci/Core.h"
#include "traci/LiteAPI.h"
#include "traci/Position.h"

namespace rover {

Define_Module(ArteryNeighbourhood);

ArteryNeighbourhood::~ArteryNeighbourhood() {
  cancelAndDelete(localMapUpdate);
  cancelAndDelete(sendMap);
}

void ArteryNeighbourhood::initialize(int stage) {
  AidBaseApp::initialize(stage);
  if (stage == INITSTAGE_LOCAL) {
    std::string x = par("coordConverterModule").stdstringValue();
    localMapUpdate = new cMessage("localMapUpdate");
    sendMap = new cMessage("sendMap");
    updateInterval = par("updateInterval").doubleValue();
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
    dMap = std::make_shared<ArteryGridDensityMap>(node_id.str(), gridSize);
    id = dMap->_nodeId;
  }
}

void ArteryNeighbourhood::setAppRequirements() {
  L3Address destAddr = chooseDestAddr();
  socket.setAppRequirements(1.0, 2.0, AidRecipientClass::ALL_LOCAL, destAddr,
                            destPort);
}

void ArteryNeighbourhood::setAppCapabilities() {
  // todo: no CAP right now.
}

void ArteryNeighbourhood::setupTimers() {
  if (destAddresses.empty()) {
    cRuntimeError("No adderss set.");
  } else {
    // schedule at startTime or current time, whatever is bigger.
    scheduleNextAppMainEvent(std::max(startTime, simTime()));
  }
}

void ArteryNeighbourhood::handleMessageWhenUp(cMessage *msg) {
  AidBaseApp::handleMessageWhenUp(msg);
}

BaseApp::FsmState ArteryNeighbourhood::fsmAppMain(cMessage *msg) {
  // send Message
  updateLocalMap();
  sendLocalMap();
  scheduleNextAppMainEvent(simTime() + updateInterval);
  return FsmRootStates::WAIT_ACTIVE;
}

void ArteryNeighbourhood::socketDataArrived(AidSocket *socket, Packet *packet) {
  auto p = checkEmitGetReceived<DensityCount>(packet);

  int numCells = p->getNumCells();
  std::string _nodeId = p->getNodeId();
  simtime_t _received = simTime();
  for (int i = 0; i < numCells; i++) {
    ArteryGridDensityMap::cellId _cId =
        std::make_pair(p->getCellX(i), p->getCellY(i));
    simtime_t _measured = p->getMTime(i);
    DensityMeasure _m(p->getCellCount(i), _measured, _received);
    dMap->updateMap(_cId, _nodeId, _m);
  }

  using namespace omnetpp;
  EV_DEBUG
      << "[ "
      << middleware->getFacilities().getConst<artery::Identity>().geonet.mid()
      << "] (YMF) ";
  dMap->printYfmMap();

  EV_DEBUG
      << "[ "
      << middleware->getFacilities().getConst<artery::Identity>().geonet.mid()
      << "] (LOCAL) ";
  dMap->printLocalMap();

  delete packet;
  socketFsmResult =
      FsmRootStates::WAIT_ACTIVE;  // GoTo WAIT_ACTIVE steady state
}

void ArteryNeighbourhood::updateLocalMap() {
  dMap->resetLocalMap();  // clear local map
  simtime_t measureTime = simTime();

  //  if (dMap->getId() == "13:5:484") simTime();

  // add yourself to the map.
  const auto &pos = middleware->getFacilities()
                        .getConst<artery::MovingNodeDataProvider>()
                        .position();
  const auto &posInet = converter_m->getConverter()->position_cast_inet(pos);

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

  const auto &table =
      middleware->getFacilities().getConst<artery::Router>().getLocationTable();
  table.visit(eVisitor);
  using namespace omnetpp;
  EV_DEBUG
      << "[ "
      << middleware->getFacilities().getConst<artery::Identity>().geonet.mid()
      << "] ";
  dMap->printLocalMap();
}

void ArteryNeighbourhood::sendLocalMap() {
  const auto &payload = createPacket<DensityCount>();
  int numValidCells = dMap->size();
  payload->setNumCells(numValidCells);
  payload->setNodeId(dMap->_nodeId.c_str());
  payload->setCellCountArraySize(numValidCells);
  payload->setCellXArraySize(numValidCells);
  payload->setCellYArraySize(numValidCells);
  payload->setMTimeArraySize(numValidCells);
  int currCell = 0;
  // get Cell count set arrays;
  ArteryGridDensityMap::view_visitor visitor =
      [&currCell, &payload](const std::pair<int, int> &cell,
                            const DensityMeasure &measure) {
        payload->setCellX(currCell, cell.first);
        payload->setCellY(currCell, cell.second);
        payload->setCellCount(currCell, measure.getCount());
        payload->setMTime(currCell, measure.getMeasureTime());
        currCell++;
      };

  dMap->visit(visitor, ArteryGridDensityMap::MapView::YMF);

  payload->setChunkLength(B(24 * currCell));  // todo calc?
  sendPayload(payload);
}

} /* namespace rover */
