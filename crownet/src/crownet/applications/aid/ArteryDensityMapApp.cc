/*
 * ArteryNeighbourhood.cc
 *
 *  Created on: Aug 21, 2020
 *      Author: sts
 */

#include "crownet/applications/aid/ArteryDensityMapApp.h"

#include <inet/common/ModuleAccess.h>
#include <memory>
#include <vanetza/geonet/areas.hpp>
#include "artery/utility/IdentityRegistry.h"
#include "omnetpp/cwatch.h"
#include "crownet/applications/PositionMapPacket_m.h"
#include "crownet/common/GlobalDensityMap.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/dcd/regularGrid/RegularDcdMapPrinter.h"
#include "traci/Core.h"
#include "traci/LiteAPI.h"
#include "traci/Position.h"

namespace crownet {

Define_Module(ArteryDensityMapApp);

ArteryDensityMapApp::~ArteryDensityMapApp() {}

void ArteryDensityMapApp::initialize(int stage) {
  AidBaseApp::initialize(stage);
  if (stage == INITSTAGE_LOCAL) {
    std::string x = par("coordConverterModule").stdstringValue();

  } else if (stage == INITSTAGE_APPLICATION_LAYER) {
    middleware = inet::findModuleFromPar<artery::Middleware>(
        par("middelwareModule"), this, true);
    identiyRegistry = inet::findModuleFromPar<artery::IdentityRegistry>(
        par("identiyRegistryModule"), this, true);
    converter = inet::getModuleFromPar<OsgCoordConverter>(
                    par("coordConverterModule"), this, true)
                    ->getConverter();

    mapType = par("mapType").stdstringValue();
    mapTypeLog = par("mapTypeLog").stdstringValue();

    // subscribe updateSignal at host module level (pedestrian, vehicle) to
    // catch identity changes.
    auto hostModule =
        middleware->getFacilities().getConst<artery::Identity>().host;
    hostModule->subscribe(artery::IdentityRegistry::updateSignal, this);
    hostId = hostModule->getId();
  }
}

void ArteryDensityMapApp::finish() {
  // remove density map for use in GlobalDensityMap context.
  emit(GlobalDensityMap::removeMap, this);
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
    RegularDcdMapFactory f{std::make_pair(gridSize, gridSize), gridDim};

    dcdMap = f.create_shared_ptr(IntIdentifer(hostId));

    if (par("writeDensityLog").boolValue()) {
      FileWriterBuilder fBuilder{};
      fBuilder.addMetadata("IDXCOL", 3);
      fBuilder.addMetadata("XSIZE", converter->getBoundaryWidth());
      fBuilder.addMetadata("YSIZE", converter->getBoundaryHeight());
      fBuilder.addMetadata("CELLSIZE", par("gridSize").doubleValue());
      fBuilder.addMetadata("MAP_TYPE", mapTypeLog);
      fBuilder.addMetadata("NODE_ID", dcdMap->getOwnerId().value());
      std::stringstream s;
      s << "dcdMap_" << hostId;
      fBuilder.addPath(s.str());

      fileWriter.reset(fBuilder.build(
          std::make_shared<RegularDcdMapValuePrinter>(dcdMap.get())));
      fileWriter->writeHeader();
    }

    // register density map for use in GlobalDensityMap context.
    emit(GlobalDensityMap::registerMap, this);
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
  if (dcdMap == nullptr)
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
  bool ret = mergeReceivedMap(packet);

  delete packet;
  socketFsmResult =
      (ret) ? FsmRootStates::WAIT_ACTIVE  // GoTo WAIT_ACTIVE steady state
            : FsmRootStates::ERR;         // GoTo ERR steady state
}

bool ArteryDensityMapApp::mergeReceivedMap(Packet *packet) {
  auto p = checkEmitGetReceived<PositionMapPacket>(packet);
  int numCells = p->getNumCells();

  int sourceNodeId = p->getNodeId();
  GridCellID _cellId = GridCellID(p->getCellId(0), p->getCellId(1));

  simtime_t _received = simTime();
  // 1) set count of all cells in local map to zero.
  // do not change the valid state.
  dcdMap->visitCells(ClearCellIdVisitor{sourceNodeId, simTime()});

  // 2) update new measurements
  for (int i = 0; i < numCells; i++) {
    GridCellID _cId{p->getCellX(i), p->getCellY(i)};
    simtime_t _measured = p->getMTime(i);
    auto _m = std::make_shared<RegularCell::entry_t>(
        p->getCellCount(i), _measured, _received, sourceNodeId);
    dcdMap->update(_cId, std::move(_m));
  }

  // 3) check local map for _nodeId and compare if the local and packet
  //    place the _nodeId in the same cell.
  dcdMap->addToNeighborhood(sourceNodeId, _cellId);

  //  using namespace omnetpp;
  //  EV_DEBUG << dMap->getView(mapType)->str();
  //  EV_DEBUG << dMap->getView("local")->str();

  return true;
}

void ArteryDensityMapApp::computeValues() {
  YmfVisitor ymf_v;  // todo make settable in config
  dcdMap->computeValues(ymf_v);
}

void ArteryDensityMapApp::updateLocalMap() {
  simtime_t measureTime = simTime();
  if (lastUpdate >= simTime()) {
    return;
  }
  lastUpdate = measureTime;

  // set count of all cells in local map to zero.
  // do not change the valid state.
  dcdMap->visitCells(ClearLocalVisitor{simTime()});
  dcdMap->clearNeighborhood();

  // add yourself to the map.
  const auto &pos = middleware->getFacilities()
                        .getConst<artery::MovingNodeDataProvider>()
                        .position();
  const auto &posInet = converter->position_cast_traci(pos);
  dcdMap->setOwnerCell(posInet);
  dcdMap->incrementLocal(posInet, dcdMap->getOwnerId(), measureTime);

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

        // increment density map
        dcdMap->incrementLocal(cartPos, _id, measureTime);
      };

  // visit
  const auto &table =
      middleware->getFacilities().getConst<artery::Router>().getLocationTable();
  table.visit(eVisitor);

  using namespace omnetpp;
  EV_DEBUG << dcdMap->strFull() << std::endl;
}

void ArteryDensityMapApp::writeMap() { fileWriter->writeData(); }

std::shared_ptr<RegularDcdMap> ArteryDensityMapApp::getMap() { return dcdMap; }

void ArteryDensityMapApp::sendMapMap() {
  const auto &payload = createPacket<PositionMapPacket>();

  computeValues();
  int numValidCells = dcdMap->valid().distance();

  payload->setNodeId(dcdMap->getOwnerId().value());
  payload->setCellId(0, dcdMap->getOwnerCell().val().first);
  payload->setCellId(1, dcdMap->getOwnerCell().val().second);

  payload->setNumCells(numValidCells);
  payload->setCellCountArraySize(numValidCells);
  payload->setCellXArraySize(numValidCells);
  payload->setCellYArraySize(numValidCells);
  payload->setMTimeArraySize(numValidCells);
  int currCell = 0;

  for (auto iter = dcdMap->valid(); iter != iter.end(); ++iter) {
    const auto &cell = (*iter).first;
    const auto &measure = (*iter).second.val();
    payload->setCellX(currCell, cell.val().first);
    payload->setCellY(currCell, cell.val().second);
    payload->setCellCount(currCell, measure->getCount());
    payload->setMTime(currCell, measure->getMeasureTime());
    currCell++;
  }

  // FIXME: real size!!!
  payload->setChunkLength(B(1000));  // todo calc: 24 * currCell Fragmentation!
  sendPayload(payload);
}

} /* namespace crownet */
