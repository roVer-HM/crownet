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
#include "crownet/crownet.h"

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

    } else if (stage == INITSTAGE_APPLICATION_LAYER){
        // BaseApp schedules start operation first (see BaseApp::initialize(stage))
        scheduleAfter(startTime, localMapTimer);
    }
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

    if (!dcdMapFactory){
        EV_WARN << "Density map factory not set. This will impact the performance because each map has a separate distance cache!" << endl;
        grid = converter->getGridDescription(par("cellSize").doubleValue());
        dcdMapFactory = std::make_shared<RegularDcdMapFactory>(grid);
    } else {
        grid = dcdMapFactory->getGrid();
    }
    
    dcdMap = dcdMapFactory->create_shared_ptr(IntIdentifer(hostId), mapCfg->getIdStreamType());
    dcdMapWatcher = new RegularDcdMapWatcher("dcdMap", dcdMap);
    WATCH_MAP(dcdMap->getNeighborhood());
    cellProvider = dcdMapFactory->getCellKeyProvider();
    // do not share valueVisitor between nodes.
    valueVisitor = dcdMapFactory->createValueVisitor(mapCfg);
}
void BaseDensityMapApp::initWriter(){
    if (mapCfg->getWriteDensityLog()) {
        ActiveFileWriterBuilder fBuilder{};
      fBuilder.addMetadata("IDXCOL", 3);
      fBuilder.addMetadata("XSIZE", grid.getGridSize().x);
      fBuilder.addMetadata("YSIZE", grid.getGridSize().y);
      fBuilder.addMetadata("XOFFSET", converter->getOffset().x);
      fBuilder.addMetadata("YOFFSET", converter->getOffset().y);
      // todo cellsize in x and y
      fBuilder.addMetadata("CELLSIZE", grid.getCellSize().x);
      fBuilder.addMetadata("MAP_TYPE", std::string(mapCfg->getMapTypeLog()));
      fBuilder.addMetadata("NODE_ID", dcdMap->getOwnerId().value());
      std::stringstream s;
      s << "dcdMap_" << hostId;
      fBuilder.addPath(s.str());

      fileWriter.reset(fBuilder.build<RegularDcdMap>(
              dcdMap, mapCfg->getMapTypeLog()));
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

void BaseDensityMapApp::applyContentTags(Ptr<Chunk> content){
    BaseApp::applyContentTags(content);
    if (par("attachEntropyTag")){
        // mark packet as Entropy Packet and not pedestrian count
        content->addTag<EntropyMap>();
    }
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
    header->setSourceId(hostId);
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
            // todo uint_16_t(cell.val() * 100) to get 1/100 count precision + check overflow!!!
            auto count_100 = cell.val()->getCount()*100;
            LocatedDcDCell c {(uint16_t)count_100, (uint16_t)0, (uint16_t)cell.getCellId().x(), (uint16_t)cell.getCellId().y()};
            auto delta_t = now-cell.val()->getMeasureTime();
            c.setDeltaCreation(delta_t);
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

  simtime_t _received = simTime();
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

      if (p->findTag<EntropyMap>() == nullptr){
          // check local map for _nodeId and compare if the local and packet
          // place the _nodeId in the same cell. For entropy maps this is not
          // applied. Because the received value does not 'move' with the sending
          // node like in the pedestrian count setup.
          dcdMap->moveNeighborTo(sourceNodeId, sourceCellId);
      }


      // update new measurements
      for (int i = 0; i < numCells; i++) {
          const LocatedDcDCell &cell = p->getCells(i);
          GridCellID entryCellId{
              baseX + cell.getIdOffsetX(),
              baseY + cell.getIdOffsetY()};
          EntryDist entryDist = cellProvider->getEntryDist(sourceCellId, dcdMap->getOwnerCell(), entryCellId);
          simtime_t _measured = cell.getCreationTime(packetCreationTime);
          if (_measured > simTime()){
              throw cRuntimeError("!!");
          }
          // get or create entry shared pointer
          auto _entry = dcdMap->getEntry<GridEntry>(entryCellId, sourceNodeId);
          _entry->setCount((double)cell.getCount()/100.0);
          _entry->setMeasureTime(_measured);
          _entry->setReceivedTime(_received);
          _entry->setEntryDist(std::move(entryDist));
          _entry->setSource(sourceNodeId);
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

void BaseDensityMapApp::setMapFactory(std::shared_ptr<RegularDcdMapFactory> factory){
    this->dcdMapFactory = factory;
}

void BaseDensityMapApp::updateOwnLocationInMap(){
    auto ownerPosition = converter->position_cast_traci(getMobility()->getCurrentPosition());
    dcdMap->setOwnerCell(ownerPosition);
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
