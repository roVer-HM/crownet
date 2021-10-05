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
#include "crownet/applications/dmap/dmap_m.h"
#include "inet/common/TimeTag_m.h"

#include <omnetpp/cwatch.h>
#include <omnetpp/cstlwatch.h>

#include "crownet/common/GlobalDensityMap.h"
#include "crownet/applications/beacon/PositionMapPacket_m.h"
#include "crownet/common/GlobalDensityMap.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/dcd/regularGrid/RegularDcdMapPrinter.h"

namespace crownet {


BaseDensityMapApp::~BaseDensityMapApp(){
    cancelAndDelete(localMapTimer);
    delete dcdMapWatcher;
    delete mapCfg;
}

void BaseDensityMapApp::initialize(int stage) {
    BaseApp::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
      mapCfg = (MapCfg*)(par("mapCfg").objectValue()->dup());
      take(mapCfg);
      hostId = getContainingNode(this)->getId();
      WATCH(hostId);

      localMapUpdateInterval = &par("localMapUpdateInterval");
      localMapTimer = new cMessage("localMapTimer");
      localMapTimer->setKind(FsmRootStates::APP_MAIN);
      scheduleAfter(localMapUpdateInterval->doubleValue(), localMapTimer);

    } else if (stage == INITSTAGE_LAST){ }
}

void BaseDensityMapApp::finish() {
  // remove density map for use in GlobalDensityMap context.
  emit(GlobalDensityMap::removeMap, this);
}

FsmState BaseDensityMapApp::handleDataArrived(Packet *packet){
    return mergeReceivedMap(packet) ? FsmRootStates::WAIT_ACTIVE  : FsmRootStates::ERR;
}

FsmState BaseDensityMapApp::fsmSetup(cMessage *msg) {

  // allow GlobalDensityMap context to set shared objects like the converter or distProvider.
  // If GlobalDensityMap is not present initDcdMap will initialize these objects
  // manually for each node. Important: This will be create multiple distProviders
  // which will affect performance.
  emit(GlobalDensityMap::initMap, this);

  initDcdMap();
  initWriter();

  // register map to GlobalDensityMap to allow synchronized logging.
  emit(GlobalDensityMap::registerMap, this);

  return BaseApp::fsmSetup(msg);
}

FsmState BaseDensityMapApp::fsmAppMain(cMessage *msg) {
  // update density map state.
  updateLocalMap();
  localMapTimer->setKind(FsmRootStates::APP_MAIN);
  scheduleAfter(localMapUpdateInterval->doubleValue(), localMapTimer);
  return FsmRootStates::WAIT_ACTIVE;
}

// App logic
void BaseDensityMapApp::initDcdMap(){
    // check if set by globalDensityMap (shared between all nodes)
    if (!converter){
        setCoordinateConverter(inet::getModuleFromPar<OsgCoordConverterProvider>(
                        par("coordConverterModule"), this)->getConverter());
    }

    std::pair<int, int> gridDim;
    double gridSize = par("gridSize").doubleValue();
    gridDim.first = floor(converter->getBoundaryWidth() / gridSize);
    gridDim.second = floor(converter->getBoundaryHeight() / gridSize);
    RegularDcdMapFactory f{std::make_pair(gridSize, gridSize), gridDim};
    
    dcdMap = f.create_shared_ptr(IntIdentifer(hostId), mapCfg->getIdStreamType());
    dcdMapWatcher = new RegularDcdMapWatcher("dcdMap", dcdMap);
    WATCH_MAP(dcdMap->getNeighborhood());
    // check if set by globalDensityMap (shared between all nodes)
    if(!distProvider) {
        distProvider = f.createDistanceProvider();
    }
    // do not share valueVisitor between nodes.
    valueVisitor = f.createValueVisitor(mapCfg->getMapType());
}
void BaseDensityMapApp::initWriter(){
    if (mapCfg->getWriteDensityLog()) {
      FileWriterBuilder fBuilder{};
      fBuilder.addMetadata("IDXCOL", 3);
      fBuilder.addMetadata("XSIZE", converter->getBoundaryWidth());
      fBuilder.addMetadata("YSIZE", converter->getBoundaryHeight());
      fBuilder.addMetadata("CELLSIZE", par("gridSize").doubleValue());
      fBuilder.addMetadata("MAP_TYPE", std::string(mapCfg->getMapTypeLog()));
      fBuilder.addMetadata("NODE_ID", dcdMap->getOwnerId().value());
      std::stringstream s;
      s << "dcdMap_" << hostId;
      fBuilder.addPath(s.str());

      fileWriter.reset(fBuilder.build<RegularDcdMap>(
              dcdMap.get(), par("mapTypeLog").stdstringValue()));
      fileWriter->writeHeader();
    }
}

const bool BaseDensityMapApp::canProducePacket(){
    // todo still produce packet (header only if no data availabel?)
    computeValues(); // Idempotent. Will only be executed once
    bool hasData = dcdMap->getCellKeyStream()->hasNext(simTime());
    if (scheduledData.get() > 0){
        // if application is scheduled based on data size
        return hasData && (scheduledData  >= minPduLength);
    } else {
        return hasData;
    }
}

const inet::b BaseDensityMapApp::getMinPdu(){
    // todo check number of occupied cells and select the corresponding header type
    return b(8*(24 + 6)); // SparseMapPacket header
}

Packet *BaseDensityMapApp::createPacket() {

    // Idempotent. Will only be executed once
    computeValues();

    // get available amout of data an calculate the number of cells
    // that can be transmitted in one packet.
    b maxData = getAvailablePduLenght();


    auto header = makeShared<MapHeader>();
    header->setSequenceNumber(localInfo->nextSequenceNumber());
    header->setVersion(MapType::SPARSE);
    header->setSourceCellIdX(dcdMap->getOwnerCell().x());
    header->setSourceCellIdY(dcdMap->getOwnerCell().y());
    header->setNumberOfNeighbours(dcdMap->getNeighborhood().size());
    maxData -= header->getChunkLength();

    // todo check map capacity and switch to DENSE Packet if needed.
    auto payload =  makeShared<SparseMapPacket>();
    maxData -= payload->getChunkLength();

    int maxCellCount = (int)(maxData.get()/payload->getCellSize().get());
    int usedSpace = 0;
    auto stream = dcdMap->getCellKeyStream();
    simtime_t now = simTime();

    payload->setCellsArraySize(maxCellCount);

    for (; usedSpace < maxCellCount; usedSpace++){
        if(stream->hasNext(now)){
            auto& cell = stream->nextCell(now);
            cell.sentAt(now);
            LocatedDcDCell c {(uint16_t)cell.val()->getCount(), (uint16_t)0, (uint16_t)cell.getCellId().x(), (uint16_t)cell.getCellId().y()};
            c.setDeltaCreation(now-cell.val()->getMeasureTime());
            payload->setCells(usedSpace, c);
        } else {
            break; // no more data present for transmission.
        }
    }
    if (usedSpace < maxCellCount ){
        payload->setCellsArraySize(usedSpace);
    }
    auto chunkLength = b(payload->getChunkLength().get() + payload->getCellsArraySize() *payload->getCellSize().get());
    payload->setChunkLength(chunkLength);

    return buildPacket(payload, header);
}

bool BaseDensityMapApp::mergeReceivedMap(Packet *packet) {

  auto header = packet->popAtFront<MapHeader>();
  if (header->getVersion() == MapType::SPARSE){
      auto p = packet->popAtFront<SparseMapPacket>();
      auto packetCreationTime = p->getTag<CreationTimeTag>()->getCreationTime();
      int numCells = p->getCellsArraySize();
      int sourceNodeId = (int)header->getSourceId();
      auto baseX = header->getRefIdOffsetX();
      auto baseY = header->getRefIdOffsetY();
      GridCellID sourceCellId = GridCellID(
              header->getSourceCellIdX(),
              header->getSourceCellIdY());

      simtime_t _received = simTime();
      // 1) set count of all cells previously received from sourceNodeId to zero.
      // do not change the valid state.
      dcdMap->visitCells(ClearCellIdVisitor{sourceNodeId, simTime()});

      // 2) check local map for _nodeId and compare if the local and packet
      //    place the _nodeId in the same cell.
      dcdMap->addToNeighborhood(sourceNodeId, sourceCellId);


      // 3) update new measurements
      for (int i = 0; i < numCells; i++) {
          const LocatedDcDCell &cell = p->getCells(i);
          GridCellID entryCellId{
              baseX + cell.getIdOffsetX(),
              baseY + cell.getIdOffsetY()};
          EntryDist entryDist = distProvider->getEntryDist(sourceCellId, dcdMap->getOwnerCell(), entryCellId);
          simtime_t _measured = cell.getCreationTime(packetCreationTime);
          auto _m = std::make_shared<RegularCell::entry_t>(
            cell.getCount(),
            _measured,
            _received,
            std::move(sourceNodeId),
            std::move(entryDist));
        dcdMap->update(entryCellId, std::move(_m));
      }


  } else if (header->getVersion() == MapType::DENSE){
      throw cRuntimeError("Not Implemented");
  } else {
      throw cRuntimeError("Wrong version.");
  }

  return true;
}

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

void BaseDensityMapApp::setCoordinateConverter(std::shared_ptr<OsgCoordinateConverter> converter){
    this->converter = converter;
}


void BaseDensityMapApp::computeValues() {
  valueVisitor->setTime(simTime());
  // dcdMap->computeValues is Idempotent
  dcdMap->computeValues(valueVisitor);
}

} // namespace crownet
