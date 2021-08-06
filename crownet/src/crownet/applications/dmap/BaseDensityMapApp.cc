//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "crownet/applications/dmap/BaseDensityMapApp.h"

#include <omnetpp/cwatch.h>
#include <omnetpp/cstlwatch.h>

#include "crownet/common/GlobalDensityMap.h"
#include "crownet/applications/beacon/PositionMapPacket_m.h"
#include "crownet/common/GlobalDensityMap.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/dcd/regularGrid/RegularDcdMapPrinter.h"

namespace crownet {

Define_Module(BaseDensityMapApp);

BaseDensityMapApp::~BaseDensityMapApp(){
    if(watcher)
        delete watcher;
}

void BaseDensityMapApp::initialize(int stage) {
    BaseApp::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
      mapType = par("mapType").stdstringValue();
      mapTypeLog = par("mapTypeLog").stdstringValue();
      hostId = getContainingNode(this)->getId();
      WATCH(hostId);

    } else if (stage == INITSTAGE_APPLICATION_LAYER) {
      converter = inet::getModuleFromPar<OsgCoordConverterProvider>(
                      par("coordConverterModule"), this)
                      ->getConverter();
    } else if (stage == INITSTAGE_LAST){
        initDcdMap();
        initWriter();
        WATCH_MAP(dcdMap->getNeighborhood());
        watcher = new RegularDcdMapWatcher("dcdMap", dcdMap);
    }
}

void BaseDensityMapApp::finish() {
  // remove density map for use in GlobalDensityMap context.
  emit(GlobalDensityMap::removeMap, this);
}

void BaseDensityMapApp::receiveSignal(cComponent *source,
                                        simsignal_t signalID, cObject *obj,
                                        cObject *details) {
    // do nothing
}

FsmState BaseDensityMapApp::handleDataArrived(Packet *packet){
    return mergeReceivedMap(packet) ? FsmRootStates::WAIT_ACTIVE  : FsmRootStates::ERR;
}

FsmState BaseDensityMapApp::fsmSetup(cMessage *msg) {
  if (dcdMap == nullptr){
      throw omnetpp::cRuntimeError(
          "Density Grid map not initialized.");
  }
  return BaseApp::fsmSetup(msg);
}

FsmState BaseDensityMapApp::fsmAppMain(cMessage *msg) {
  // send Message
  updateLocalMap();
  sendMapMap();
  scheduleNextAppMainEvent();
  return FsmRootStates::WAIT_ACTIVE;
}

// App logic
void BaseDensityMapApp::initDcdMap(){
    std::pair<int, int> gridDim;
    double gridSize = par("gridSize").doubleValue();
    gridDim.first = floor(converter->getBoundaryWidth() / gridSize);
    gridDim.second = floor(converter->getBoundaryWidth() / gridSize);
    RegularDcdMapFactory f{std::make_pair(gridSize, gridSize), gridDim};

    dcdMap = f.create_shared_ptr(IntIdentifer(hostId));
    // if global is present will be overwritten to share one provider between all maps
    distProvider = f.createDistanceProvider();
    valueVisitor = f.createValueVisitor(mapType);
}
void BaseDensityMapApp::initWriter(){
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

      fileWriter.reset(fBuilder.build<RegularDcdMap>(
              dcdMap.get(), par("mapTypeLog").stdstringValue()));
      fileWriter->writeHeader();
    }

    // register density map for use in GlobalDensityMap context.
    emit(GlobalDensityMap::registerMap, this);
}

void BaseDensityMapApp::sendMapMap() {

  // todo calc: 24 * currCell Fragmentation!
  // FIXME: real size!!!
  const auto &payload = createPacket<PositionMapPacket>(B(1000));

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

  sendPayload(payload);
}

bool BaseDensityMapApp::mergeReceivedMap(Packet *packet) {
  auto p = packet->popAtFront<PositionMapPacket>();
  int numCells = p->getNumCells();

  int sourceNodeId = p->getNodeId();
  GridCellID sourceCellId = GridCellID(p->getCellId(0), p->getCellId(1));

  simtime_t _received = simTime();
  // 1) set count of all cells in local map to zero.
  // do not change the valid state.
  dcdMap->visitCells(ClearCellIdVisitor{sourceNodeId, simTime()});

  // 2) update new measurements
  for (int i = 0; i < numCells; i++) {
    GridCellID entryCellId{p->getCellX(i), p->getCellY(i)};
    EntryDist entryDist = distProvider->getEntryDist(sourceCellId, dcdMap->getOwnerCell(), entryCellId);
    simtime_t _measured = p->getMTime(i);
    auto _m = std::make_shared<RegularCell::entry_t>(
        p->getCellCount(i), _measured, _received, std::move(sourceNodeId), std::move(entryDist));
    dcdMap->update(entryCellId, std::move(_m));
  }

  // 3) check local map for _nodeId and compare if the local and packet
  //    place the _nodeId in the same cell.
  dcdMap->addToNeighborhood(sourceNodeId, sourceCellId);
  //  using namespace omnetpp;
  //  EV_DEBUG << dMap->getView(mapType)->str();
  //  EV_DEBUG << dMap->getView("local")->str();

  return true;
}

//

void BaseDensityMapApp::updateLocalMap() {
    throw omnetpp::cRuntimeError("Not Implemented in Base* class. Use child class");
}

void BaseDensityMapApp::writeMap() {
    fileWriter->writeData();
}

std::shared_ptr<RegularDcdMap> BaseDensityMapApp::getMap() { return dcdMap; }

void BaseDensityMapApp::setDistanceProvider(std::shared_ptr<GridCellDistance> distProvider){
    this->distProvider = distProvider;
}


void BaseDensityMapApp::computeValues() {
  valueVisitor->setTime(simTime());
  dcdMap->computeValues(valueVisitor);
}

} // namespace crownet
