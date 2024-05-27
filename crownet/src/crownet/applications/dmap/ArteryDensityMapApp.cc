/*
 * ArteryNeighbourhood.cc
 *
 *  Created on: Aug 21, 2020
 *      Author: sts
 */

#include "crownet/applications/dmap/ArteryDensityMapApp.h"

#include <inet/common/ModuleAccess.h>
#include <memory>
#include <vanetza/geonet/areas.hpp>
#include "artery/utility/IdentityRegistry.h"
#include "traci/Core.h"
#include "traci/Position.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"

namespace crownet {

Define_Module(ArteryDensityMapApp);

void ArteryDensityMapApp::initialize(int stage) {
    BaseDensityMapApp::initialize(stage);
    if (stage == INITSTAGE_APPLICATION_LAYER) {
        middleware = inet::getModuleFromPar<artery::Middleware>(
            par("middelwareModule"), this);
        identiyRegistry = inet::getModuleFromPar<artery::IdentityRegistry>(
            par("identiyRegistryModule"), this);
        mRouter = inet::getModuleFromPar<artery::Router>(par("routerModule"), this);
        mapDataType = DpmmMapType::PEDESTRIAN_COUNT;


        // subscribe updateSignal at host module level (pedestrian, vehicle) to
        // catch identity changes. (hostId) should be stable!
        auto hostModule =
            middleware->getFacilities().getConst<artery::Identity>().host;
        hostModule->subscribe(artery::IdentityRegistry::updateSignal, this);
    }
}

void ArteryDensityMapApp::receiveSignal(cComponent *source,
                                        simsignal_t signalID, cObject *obj,
                                        cObject *details) {
  if (signalID == artery::IdentityRegistry::updateSignal) {
    int _hostId = middleware->getFacilities()
        .getConst<artery::Identity>()
        .host->getId();
    if (_hostId != hostId){
        // todo handle id update for density map
        throw omnetpp::cRuntimeError("hostId mismatch.");
    }
  }
}


FsmState ArteryDensityMapApp::fsmSetup(cMessage *msg) {
  return BaseDensityMapApp::fsmSetup(msg);
}

FsmState ArteryDensityMapApp::fsmAppMain(cMessage *msg) {
  // update density map state.
  updateLocalMap();
  mainAppTimer->setKind(FsmRootStates::APP_MAIN);
  scheduleAfter(mainAppInterval->doubleValue(), mainAppTimer);
  return FsmRootStates::WAIT_ACTIVE;
}

void ArteryDensityMapApp::updateLocalMap() {
  simtime_t measureTime = simTime();
  if (lastUpdate >= simTime()) {
    return;
  }
  lastUpdate = measureTime;

  // set count of all cells in local map to zero.
  // do not change the valid state.
  ClearLocalVisitor v;
  v.setTime(simTime());
  dcdMap->visitCells(std::move(v));
  dcdMap->clearNeighborhood();

  // add yourself to the map.
  const auto &pos = middleware->getFacilities()
                        .getConst<artery::MovingNodeDataProvider>()
                        .position();
  const auto &posInet = converter->position_cast_traci(pos);
  dcdMap->setOwnerCell(posInet);
  dcdMap->getEntry<GridEntry>(posInet)->incrementCount(measureTime, 1.0);

  // visitor for artery location table
  vanetza::geonet::LocationTable::entry_visitor eVisitor =
      [this, &measureTime](const vanetza::MacAddress &mac,
                           const vanetza::geonet::LocationTableEntry &entry) {
        // only entries with valid position vector
        if (!entry.has_position_vector()) return;

        auto identity =
            identiyRegistry->lookup<artery::IdentityRegistry::mac>(mac);
        if (!identity) {
          EV_DEBUG << "No Identity for MAC address " << mac
                   << "found. (left simulaiton)";
          return;
        }

        int _id = identity->host->getId();

        // convert Geo to 2D-Cartesian
        const vanetza::geonet::GeodeticPosition geoPos =
            entry.get_position_vector().position();
        auto cartPos = converter->convertToCartTraCIPosition(geoPos);

        // Update cell entries in dcdMap:
        // Assume all beacons carry '1' as value (1 beacon == 1 count)
        // Pedestrian count beacons are additive thus incremnt is possible.
        if (dcdMap->isInNeighborhood(_id)){
            auto oldCell = dcdMap->getNeighborCell(_id);
            dcdMap->getEntry<GridEntry>(oldCell)->decrementCount(measureTime, 1.0);
        }
        dcdMap->addToNeighborhood(_id, cartPos);
        dcdMap->getEntry<GridEntry>(cartPos)->incrementCount(measureTime, 1.0);
      };

  // visit
  const auto &table = mRouter->getLocationTable();
//      middleware->getFacilities().getConst<artery::Router>().getLocationTable();
  table.visit(eVisitor);

  using namespace omnetpp;
  EV_DEBUG << dcdMap->strFull() << std::endl;
}




} /* namespace crownet */
