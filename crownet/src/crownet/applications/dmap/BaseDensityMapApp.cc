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
#include "crownet/dcd/regularGrid/RegularDcdMapPrinter.h"
#include "crownet/crownet.h"

namespace crownet {


BaseDensityMapApp::~BaseDensityMapApp(){
    cancelAndDelete(mainAppTimer);
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

      mainAppInterval = &par("mainAppInterval");
      mainAppTimer = new cMessage("mainAppTimer");
      mainAppTimer->setKind(FsmRootStates::APP_MAIN);
      mapDataType = "pedestrianCount";

    } else if (stage == INITSTAGE_APPLICATION_LAYER){
        // BaseApp schedules start operation first (see BaseApp::initialize(stage))
        if (mainAppInterval->doubleValue() > 0.){
            scheduleAfter(startTime, mainAppTimer);
        } else {
            EV << "mainAppTimer deactivated." << endl;
        }
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
  EV << "BaseDensityMapApp::fsmAppMain - do nothing" << endl;
  mainAppTimer->setKind(FsmRootStates::APP_MAIN);
  scheduleAfter(mainAppInterval->doubleValue(), mainAppTimer);
  return FsmRootStates::WAIT_ACTIVE;
}

// App logic
void BaseDensityMapApp::initDcdMap(){
    // check if set by globalDensityMap (shared between all nodes)
    if (!converter){
        auto _c = inet::getModuleFromPar<OsgCoordConverterProvider>(
                par("coordConverterModule"), this)
                ->getConverter();
        auto cellSize = par("cellSize").doubleValue();
        if (_c->getCellSize() != inet::Coord(cellSize, cellSize)){
            throw cRuntimeError("cellSize mismatch between converter and density map. Converter [%f, %f] vs map %f",
                    converter->getCellSize().x, converter->getCellSize().y, cellSize
            );
        }
        setCoordinateConverter(_c);
    }

    if (!dcdMapFactory){
        EV_WARN << "Density map factory not set. This will impact the performance because each map has a separate distance cache!" << endl;
        dcdMapFactory = std::make_shared<RegularDcdMapFactory>(converter);
    }
    
    dcdMap = dcdMapFactory->create_shared_ptr(IntIdentifer(hostId), mapCfg->getIdStreamType());
    dcdMapWatcher = new RegularDcdMapWatcher("dcdMap", dcdMap);
    WATCH_MAP(dcdMap->getNeighborhood());
    cellProvider = dcdMapFactory->getCellKeyProvider();
    // do not share valueVisitor between nodes.
    valueVisitor = dcdMapFactory->createValueVisitor(mapCfg);
    cellAgeHandler = std::make_shared<TTLCellAgeHandler>(dcdMap, mapCfg->getCellAgeTTL(), simTime());
    if (mapCfg->getAppendRessourceSharingDomoinId()){
        rsdVisitor = std::make_shared<ApplyRessourceSharingDomainIdVisitor>(simTime());
    }


}
void BaseDensityMapApp::initWriter(){
    if (mapCfg->getWriteDensityLog()) {
      ActiveFileWriterBuilder fBuilder{};
      fBuilder.addMetadata("IDXCOL", 3);
      fBuilder.addMetadata("XSIZE", converter->getGridSize().x);
      fBuilder.addMetadata("YSIZE", converter->getGridSize().y);
      fBuilder.addMetadata("XOFFSET", converter->getOffset().x);
      fBuilder.addMetadata("YOFFSET", converter->getOffset().y);
      fBuilder.addMetadata<const traci::Boundary&>("SIM_BBOX", converter->getSimBound());
      // todo cellsize in x and y
      fBuilder.addMetadata("CELLSIZE", converter->getCellSize().x);
      fBuilder.addMetadata("VERSION", std::string("0.4")); // todo!!!
      fBuilder.addMetadata("DATATYPE", mapDataType);
      fBuilder.addMetadata("MAP_TYPE", std::string(mapCfg->getMapTypeLog()));
      fBuilder.addMetadata("NODE_ID", dcdMap->getOwnerId().value());
      std::stringstream s;
      s << "dcdMap_" << hostId;
      fBuilder.addPath(s.str());

      fileWriter.reset(fBuilder.build<RegularDcdMap>(
              dcdMap, mapCfg));
    } else {
        // do nothing
        fileWriter.reset(new DevNullWriter());
    }

    fileWriter->initWriter();


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

const inet::b BaseDensityMapApp::getMinPdu() const {
    // todo check number of occupied cells and select the corresponding header type
    return b(8*(24 + 6)); // SparseMapPacket header
}

BurstInfo BaseDensityMapApp::getBurstInfo(inet::b scheduled) const{
    MapHeader h;
    SparseMapPacket p;
    int max_cells_per_pkt = ((getMaxPdu() - h.getChunkLength())/p.getCellSize()).get();

    int num_cells_available = dcdMap->getCellKeyStream()->size(simTime());

    int num_pkt_needed = (int)std::ceil((double)num_cells_available/max_cells_per_pkt);
    int num_pkt_possible = (int)std::ceil(((double)scheduled.get()/getMaxPdu().get()));

    inet::b burst_size;
    int pkt_num;
    if (num_pkt_needed <= num_pkt_possible){
        // send num_pkt_needed *NEEDED* packets where the last one is most likly not full.
        // Thus pkt_num times header plus all cells available.
        burst_size = inet::b(num_pkt_needed*h.getChunkLength().get() + num_cells_available*p.getCellSize().get());
        pkt_num = num_pkt_needed;
    } else {
        // send num_pkt_possible *FULL* packets
        burst_size = inet::b(num_pkt_possible*(h.getChunkLength().get() + max_cells_per_pkt*p.getCellSize().get()));
        pkt_num = num_pkt_possible;
    }
    return BurstInfo{pkt_num, burst_size};
}



Ptr<Chunk>  BaseDensityMapApp::buildHeader(){
    auto header = makeShared<MapHeader>();
    auto seqNo = localInfo->nextSequenceNumber();
    header->setSequenceNumber(seqNo);
    header->addTagIfAbsent<SequenceIdTag>()->setSequenceNumber(seqNo);
    header->setSourceId(hostId);
    if (mapCfg->getAppendRessourceSharingDomoinId()){
        header->setVersion(MapType::SPARSE_RSD);
    } else {
        header->setVersion(MapType::SPARSE);
    }

    header->setCellIdOffsetX(0);
    header->setCellIdOffsetY(0);
    header->setSourcePosition(getPosition());
    return header;
}

Ptr<Chunk>  BaseDensityMapApp::buildPayload(b maxData){
    if (mapCfg->getAppendRessourceSharingDomoinId()){
        return buildPayload(maxData, makeShared<SparseMapPacketWithSharingDomainId>());
    } else {
        return buildPayload(maxData, makeShared<SparseMapPacket>());
    }
}

Ptr<Chunk>  BaseDensityMapApp::buildPayload(b maxData, Ptr<SparseMapPacket> payload){
    // todo check map capacity and switch to DENSE Packet if needed.
    maxData -= payload->getChunkLength();

    int maxCellCount;
    int cellSize;
    maxCellCount = (int)(maxData.get()/payload->getCellSize().get());
    cellSize = payload->getCellSize().get();

    int usedSpace = 0;
    auto stream = dcdMap->getCellKeyStream();
    simtime_t now = simTime();

    payload->setCellsArraySize(maxCellCount);

    for (; usedSpace < maxCellCount; usedSpace++){
        if(!stream->hasNext(now)){
            break; // no more data present for transmission.
        }
        auto& cell = stream->nextCell(now);
        cell.sentAt(now);
        auto count_100 = cell.val()->getCount()*100;

        LocatedDcDCell c {
            (uint16_t)count_100,    //count
            (uint16_t)cell.getCellId().x(), // offsetX
            (uint16_t)cell.getCellId().y()  // offsetY
        };
        auto delta_t = now-cell.val()->getMeasureTime();
        c.setDeltaCreation(delta_t);
        c.setSourceEntryDist(cell.val()->getEntryDist().sourceEntry); // todo size
        payload->setCells(usedSpace, c);
    }

    if (usedSpace < maxCellCount ){
        payload->setCellsArraySize(usedSpace);
    }
    auto chunkLength = b(payload->getChunkLength().get() + payload->getCellsArraySize() *cellSize);
    payload->setChunkLength(chunkLength);
    return payload;
}

Ptr<Chunk>  BaseDensityMapApp::buildPayload(b maxData, Ptr<SparseMapPacketWithSharingDomainId> payload){
    // todo check map capacity and switch to DENSE Packet if needed.
    maxData -= payload->getChunkLength();

    int maxCellCount;
    int cellSize;
    maxCellCount = (int)(maxData.get()/payload->getCellSize().get());
    cellSize = payload->getCellSize().get();

    int usedSpace = 0;
    auto stream = dcdMap->getCellKeyStream();
    simtime_t now = simTime();

    payload->setCellsArraySize(maxCellCount);

    for (; usedSpace < maxCellCount; usedSpace++){
        if(!stream->hasNext(now)){
            break; // no more data present for transmission.
        }

        auto& cell = stream->nextCell(now);
        cell.sentAt(now);
        auto count_100 = cell.val()->getCount()*100;

        LocatedDcDCellWithSharingDomainId c {
            (uint16_t)count_100,    //count
            (uint16_t)cell.getCellId().x(), // offsetX
            (uint16_t)cell.getCellId().y(),  // offsetY
            cell.val()->getResourceSharingDomainId()
        };
        auto delta_t = now-cell.val()->getMeasureTime();
        c.setDeltaCreation(delta_t);
        c.setSourceEntryDist(cell.val()->getEntryDist().sourceEntry); // todo size
        payload->setCells(usedSpace, c);
    }

    if (usedSpace < maxCellCount ){
        payload->setCellsArraySize(usedSpace);
    }
    auto chunkLength = b(payload->getChunkLength().get() + payload->getCellsArraySize() *cellSize);
    payload->setChunkLength(chunkLength);
    return payload;
}



Packet *BaseDensityMapApp::createPacket() {

    // Idempotent. Will only be executed once
    computeValues();

    // get available amout of data an calculate the number of cells
    // that can be transmitted in one packet.
    b maxData = getAvailablePduLenght();

    auto header = buildHeader();
    maxData -= header->getChunkLength();

    auto payload = buildPayload(maxData);

    return buildPacket(payload, header);
}

bool BaseDensityMapApp::mergeReceivedMap(Packet *packet) {

  auto header = packet->popAtFront<MapHeader>();
  if (header->getSourceId() == getHostId()){
      // self map packet. ignore it
      EV_INFO << getHostId() << "received own density map. Ignore it." << endl;
      return true;
  }
  bool ret = false;
  switch (header->getVersion()){
  case MapType::SPARSE:
      ret = mergeReceivedMap(header, packet->popAtFront<SparseMapPacket>());
      break;
  case MapType::SPARSE_RSD:
      ret = mergeReceivedMap(header, packet->popAtFront<SparseMapPacketWithSharingDomainId>());
      break;
  default:
      throw cRuntimeError("Map version '%i' not implemented", header->getVersion());
  }

  return ret;
}

bool BaseDensityMapApp::mergeReceivedMap(Ptr<const MapHeader> header, const Ptr<const SparseMapPacket> body){
    simtime_t _received = simTime();
    auto packetCreationTime = body->getTag<CreationTimeTag>()->getCreationTime();


    int numCells = body->getCellsArraySize();
    int sourceNodeId = (int)header->getSourceId();
    auto baseX = header->getCellIdOffsetX();
    auto baseY = header->getCellIdOffsetY();

    auto sourcePosition = converter->position_cast_traci(header->getSourcePosition());
    GridCellID sourceCellId = dcdMap->getCellId(sourcePosition);
    Coord senderPosition = header->getSourcePosition();

    // update new measurements
    for (int i = 0; i < numCells; i++) {
        const LocatedDcDCell &cell = body->getCells(i);
        GridCellID entryCellId{
            baseX + cell.getIdOffsetX(),
            baseY + cell.getIdOffsetY()};
        /**
         *  extract sourceEntryDist from packet. This distance is the distance
         *  from which the Entry was generated by the
         *  original 'node'. The sender might be the original node but does not
         *  have to be. Further more the
         *  sender might have moved between measuring and sending the value.
         *  Other distances (i.e. hostEntry, sourceHost) must be calculated.
         */
        EntryDist entryDist = cellProvider->getExactDist(senderPosition, getPosition(), entryCellId, cell.getSourceEntryDist());
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

    return true;
}

bool BaseDensityMapApp::mergeReceivedMap(Ptr<const MapHeader> header, const Ptr<const SparseMapPacketWithSharingDomainId> body){
    simtime_t _received = simTime();
    auto packetCreationTime = body->getTag<CreationTimeTag>()->getCreationTime();


    int numCells = body->getCellsArraySize();
    int sourceNodeId = (int)header->getSourceId();
    auto baseX = header->getCellIdOffsetX();
    auto baseY = header->getCellIdOffsetY();

    auto sourcePosition = converter->position_cast_traci(header->getSourcePosition());
    GridCellID sourceCellId = dcdMap->getCellId(sourcePosition);
    Coord senderPosition = header->getSourcePosition();

    // update new measurements
    for (int i = 0; i < numCells; i++) {
        const LocatedDcDCellWithSharingDomainId &cell = body->getCells(i);
        GridCellID entryCellId{
            baseX + cell.getIdOffsetX(),
            baseY + cell.getIdOffsetY()};
        /**
         *  extract sourceEntryDist from packet. This distance is the distance
         *  from which the Entry was generated by the
         *  original 'node'. The sender might be the original node but does not
         *  have to be. Further more the
         *  sender might have moved between measuring and sending the value.
         *  Other distances (i.e. hostEntry, sourceHost) must be calculated.
         */
        EntryDist entryDist = cellProvider->getExactDist(senderPosition, getPosition(), entryCellId, cell.getSourceEntryDist());
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
        _entry->setResourceSharingDomainId(cell.getSharingDominId());
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
   simtime_t now = simTime();
   // cellAgeHandler is Idempotent
  cellAgeHandler->setTime(now);
  dcdMap->visitCells(*cellAgeHandler); //reference to cellAgeHandler needed


  valueVisitor->setTime(now);
  // dcdMap->computeValues is Idempotent
  dcdMap->computeValues(valueVisitor);

  if (mapCfg->getAppendRessourceSharingDomoinId()){
      rsdVisitor->reset(now, getResourceSharingDomainId());
      dcdMap->visitCells(*rsdVisitor); //reference to cellAgeHandler needed
  }
}



} // namespace crownet
