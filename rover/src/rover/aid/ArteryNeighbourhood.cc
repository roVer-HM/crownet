/*
 * ArteryNeighbourhood.cc
 *
 *  Created on: Aug 21, 2020
 *      Author: sts
 */

#include "ArteryNeighbourhood.h"
#include <inet/common/ModuleAccess.h>
#include <vanetza/geonet/areas.hpp>
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
  cSimpleModule::initialize(stage);
  if (stage == INITSTAGE_LOCAL) {
    std::string x = par("coordConverterModule").stdstringValue();
    localMapUpdate = new cMessage("localMapUpdate");
    sendMap = new cMessage("sendMap");
    updateInterval = par("updateInterval").doubleValue();
    gridSize = par("gridSize").doubleValue();
    scheduleAt(simTime() + updateInterval, localMapUpdate);
    scheduleAt(simTime() + 0.1 + updateInterval, sendMap);
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

void ArteryNeighbourhood::handleMessage(cMessage *message) {
  if (message == localMapUpdate) {
    updateLocalMap();
    scheduleAt(simTime() + updateInterval, localMapUpdate);
  } else if (message == sendMap) {
  } else {
    throw cRuntimeError("ArteryNeighbourhood only handles self messages.");
  }
}

void ArteryNeighbourhood::updateLocalMap() {
  dMap->resetLocalMap();  // clear local map
  simtime_t measureTime = simTime();

  if (dMap->getId() == "13:5:484") simTime();

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
  // todo: get local map and send information as Broadcast
}

void ArteryNeighbourhood::receiveMapUpdate(cMessage *pkt) {
  // todo: read packet and update map with measures.
  delete pkt;
}

} /* namespace rover */
