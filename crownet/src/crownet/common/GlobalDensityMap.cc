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

#include "crownet/common/GlobalDensityMap.h"

#include <inet/common/ModuleAccess.h>
#include <inet/mobility/contract/IMobility.h>
#include <omnetpp/checkandcast.h>
#include "artery/application/MiddlewareBase.h"
#include "artery/application/MovingNodeDataProvider.h"
#include "artery/utility/Identity.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/dcd/regularGrid/MapCellAggregationAlgorithms.h"
#include "crownet/dcd/regularGrid/RegularDcdMapPrinter.h"

namespace {
const simsignal_t traciConnected = cComponent::registerSignal("traci.connected");
}  // namespace

namespace crownet {

Define_Module(GlobalDensityMap);

const simsignal_t GlobalDensityMap::initMap =
        cComponent::registerSignal("InitDensityMap");
const simsignal_t GlobalDensityMap::registerMap =
    cComponent::registerSignal("RegisterDensityMap");
const simsignal_t GlobalDensityMap::removeMap =
    cComponent::registerSignal("RemoveDensityMap");
const simsignal_t GlobalDensityMap::registerNodeAcceptor =
    cComponent::registerSignal("RegisterNodeAcceptor");

GlobalDensityMap::~GlobalDensityMap() {
  if (writeMapTimer) {
      cancelAndDelete(writeMapTimer);
  }
  if (appDeleteNode){
      cancelAndDelete(appDeleteNode);
  }
  delete mapCfg;
}

void GlobalDensityMap::initialize() {
    getSystemModule()->subscribe(initMap, this);
  getSystemModule()->subscribe(registerMap, this);
  getSystemModule()->subscribe(removeMap, this);
  getSystemModule()->subscribe(registerNodeAcceptor, this);
  // listen to traciConnected signal to setup density map *before*
  // subscriptions and node are initialized.
  getSystemModule()->subscribe(traciConnected, this);
}

void GlobalDensityMap::finish() {
  getSystemModule()->unsubscribe(registerMap, this);
  getSystemModule()->unsubscribe(removeMap, this);
  getSystemModule()->unsubscribe(traciConnected, this);
  fileWriter->finish();
}

void GlobalDensityMap::receiveSignal(omnetpp::cComponent *source,
                                     omnetpp::simsignal_t signalId,
                                     omnetpp::cObject *obj,
                                     omnetpp::cObject *details) {
  if (signalId == initMap){
      auto mapHandler = check_and_cast<GridHandler *>(obj);
      mapHandler->setMapFactory(dcdMapFactory);
      mapHandler->setCoordinateConverter(converter);

      if (par("writerType").stdstringValue() == "sql"){
          // only share the sql writer between maps
          // todo mw
          // mapHandler->setSqlApi(api);
      }
  }
  else if (signalId == registerMap) {
    auto mapHandler = check_and_cast<GridHandler *>(obj);
    dezentralMaps[mapHandler->getMap()->getOwnerId()] = mapHandler;
    EV_DEBUG << "register DensityMap for node: "
             << mapHandler->getMap()->getOwnerId();

  } else if (signalId == removeMap) {
    auto mapHandler = check_and_cast<GridHandler *>(obj);
    dezentralMaps.erase(mapHandler->getMap()->getOwnerId());
    EV_DEBUG << "remove DensityMap for node: "
             << mapHandler->getMap()->getOwnerId();
  } else if (signalId == registerNodeAcceptor){
      auto acceptor = check_and_cast<ITraCiNodeVisitorAcceptor*>(obj);
      dynamicNodeVisitorAcceptors.push_back(acceptor);
  }
}
void GlobalDensityMap::receiveSignal(cComponent *source, simsignal_t signalID,
                                     const SimTime &t, cObject *details) {
  if (signalID == traciConnected) {
      initializeMap();
  }
}

void GlobalDensityMap::initializeMap(){
    // 1) setup map
    converter = inet::getModuleFromPar<OsgCoordConverterProvider>(
                    par("coordConverterModule"), this)
                    ->getConverter();
    auto cellSize = par("cellSize").doubleValue();
    if (converter->getCellSize() != inet::Coord(cellSize, cellSize)){
        throw cRuntimeError("cellSize mismatch between converter and density map. Converter [%f, %f] vs map %f",
                converter->getCellSize().x, converter->getCellSize().y, cellSize
        );
    }

    dcdMapFactory = std::make_shared<RegularDcdMapFactory>(converter);

    dcdMapGlobal = dcdMapFactory->create_shared_ptr(IntIdentifer(-1));  // global
    cellKeyProvider = dcdMapFactory->getCellKeyProvider();

    // 2) setup writer.
    if (par("writerType").stdstringValue() == "csv"){
        ActiveFileWriterBuilder fBuilder{};
        fBuilder.addMetadata("IDXCOL", 3);
        fBuilder.addMetadata("XSIZE", converter->getGridSize().x);
        fBuilder.addMetadata("YSIZE", converter->getGridSize().y);
        fBuilder.addMetadata("XOFFSET", converter->getOffset().x);
        fBuilder.addMetadata("YOFFSET", converter->getOffset().y);
        // todo cellsize in x and y
        fBuilder.addMetadata("CELLSIZE", converter->getCellSize().x);
        fBuilder.addMetadata("VERSION", std::string("0.4")); // todo!!!
        fBuilder.addMetadata("DATATYPE", mapDataType);
        fBuilder.addMetadata<std::string>(
            "MAP_TYPE",
            mapCfg->getMapType());  // The global density map is the ground
                                    // truth. No algorihm needed.
        fBuilder.addMetadata<const traci::Boundary&>("SIM_BBOX", converter->getSimBound());
        fBuilder.addMetadata<int>("NODE_ID", -1);
        fBuilder.addPath("global");

        fileWriter.reset(fBuilder.build(
            std::make_shared<RegularDcdMapGlobalPrinter>(dcdMapGlobal)));
    } else if (par("writerType").stdstringValue() == "sql"){
//      todo mw
//
//      create sqlApi <--- will be shared
//      create sqlWriter/Printer for global map
//          todo mw setup SqlLiteWriter
//          auto sqlWriter = std::make_shared<SqlLiteWriter>();
//          auto sqlPrinter = std::make_shared<RegularDcdMapSqlValuePrinter>(dcdMapGlobal);
//          sqlWriter->setSqlApi(sqlApi);
//          sqlWriter->setPrinter(sqlPrinter);
//          filewriter = sqlWriter;
    } else {
        throw cRuntimeError("expected sql or csv as writer type got '%s'", par("writerType").stringValue());
    }
    fileWriter->initWriter();
}


void GlobalDensityMap::initialize(int stage) {
  cSimpleModule::initialize(stage);
  if (stage == INITSTAGE_LOCAL) {
      // setup ground truth search locations
      // i.e single nodes, vector nodes not managed by TraCI, or TraCI managed vectors.
      // other sources can be registered over the signal registerNodeAcceptor
      cStringTokenizer tt(par("nodeModules").stringValue(), ";");
      singleNodeModules = tt.asVector();

      vectorNodeModule = par("vectorNodeModule").stdstringValue();
      if (vectorNodeModule != ""){
          dynamicNodeVisitorAcceptors.push_back(this);
      }
      mapDataType = "pedestrianCount";
      mapCfg = (MapCfg*)(par("mapCfg").objectValue()->dup());
      take(mapCfg);

      cModule* _traciModuleListener = findModuleByPath(par("traciModuleListener").stringValue());
      if (_traciModuleListener){
          auto acceptor = check_and_cast<ITraCiNodeVisitorAcceptor*>(_traciModuleListener);
          dynamicNodeVisitorAcceptors.push_back(acceptor);
      }
      if (par("deleteTime").doubleValue() > 0.0){
          appDeleteNode = new cMessage("deleteNode");
          scheduleAt(par("deleteTime").doubleValue(), appDeleteNode);
      }

  } else if (stage == INITSTAGE_APPLICATION_LAYER) {
    m_mobilityModule = par("mobilityModule").stdstringValue();

    writeMapTimer = new cMessage("GlobalDensityMapTimer");
    writeMapInterval = par("writeMapInterval").doubleValue();
    if (writeMapInterval > 0) {
      scheduleAt(simTime() + writeMapInterval, writeMapTimer);
    }

    // todo may be set via ini file
    valueVisitor = std::make_shared<LocalSelector>(simTime());
  } else if ( stage == INITSTAGE_LAST){
      if (!par("useSignalMapInit").boolValue()){
          initializeMap();
      }
  }
}

void GlobalDensityMap::acceptTraciVisitor(traci::ITraciNodeVisitor* visitor){
    // check nodes NOT managed by TraCI
       // only vector 'misc' is supported.
       cModule* root = findModuleByPath("<root>");
       cModule* m = root->getSubmodule(vectorNodeModule.c_str(), vectorNodeIndex);
       if (m){
           if (m->isVector()){
              for(int i = vectorNodeIndex; i < m->getVectorSize(); i++){
                  cModule* mm = root->getSubmodule("misc", i);
                  if (mm){
                      visitor->visitNode("", mm);
                  }
              }
           } else {
               throw cRuntimeError("expected vector node with name 'misc'");
           }
       }
}

void GlobalDensityMap::acceptNodeVisitor(traci::ITraciNodeVisitor* visitor){


    for(const auto& path: singleNodeModules){
        cModule* m = findModuleByPath(path.c_str());
        if (m){
            visitor->visitNode("", m);
        }
    }
    // call all acceptors registered. (i.e. traci, bonn motion server, or this in case of static nodes)
    for (auto acceptor : dynamicNodeVisitorAcceptors){
        acceptor->acceptTraciVisitor(visitor);
    }

}


void GlobalDensityMap::handleMessage(cMessage *msg) {
  if (msg == writeMapTimer) {
    // 1) update maps
    updateMaps();

    // 2) write to file
    writeMaps();

    // 3) reschedule
    scheduleAt(simTime() + writeMapInterval, msg);
  }  else if (msg == appDeleteNode) {
      //hack: Allows the 'one time deletion' of nodes in the vector from the beginning.
      //misc_base_index will point to the 'new' first element. The vector size is not changed
      //by this deletion.
      int removeCount = std::floor(dezentralMaps.size() * par("deletePercentage").doubleValue());
      vectorNodeIndex = removeCount;
      std::vector<cModule*> nodes;
      for (const auto& e : dezentralMaps){
          if (removeCount <= 0){
              break;
          }
          cModule* m = e.second->getModule();
          nodes.push_back(getContainingNode(m));
          --removeCount;
      }
      for (auto n: nodes){
          n->callFinish();  //ensures 'normal' shutdown and unregistering of removed node
          n->deleteModule();
      }
      cancelAndDelete(appDeleteNode);
      appDeleteNode = nullptr;
  }  else {
    delete msg;
  }
}

void GlobalDensityMap::visitNode(const std::string& traciNodeId, omnetpp::cModule* mod) {
  const auto mobility = check_and_cast<inet::IMobility*>(mod->getModuleByPath(m_mobilityModule.c_str()));
  // convert to traci 2D position
  const auto &pos = mobility->getCurrentPosition();
  const auto &posInet = converter->position_cast_traci(pos);

  // visitNode is called for *all* nodes thus this 'local' map of the global
  // module represents the global (ground truth) of the simulation.
  auto e = dcdMapGlobal->getEntry<GridGlobalEntry>(posInet);
  e->incrementCount(simTime()); // increment by 1
  e->nodeIds.insert(mod->getId());
}

/**
 * update global and decentralized maps together to get the same time stamp.
 */
void GlobalDensityMap::updateMaps() {
  // update global map. (Visit all nodes managed by traci)
  // only update once for given simtime_t
  if (lastUpdate >= simTime()) {
    return;
  }
  lastUpdate = simTime();

  // global map needs reset (not clear)
  dcdMapGlobal->visitCells(ResetVisitor{lastUpdate});
  dcdMapGlobal->clearNeighborhood();
  acceptNodeVisitor(this);
  valueVisitor->setTime(simTime());
  dcdMapGlobal->computeValues(valueVisitor);

  // update decentralized map
  for (auto &handler : dezentralMaps) {
    handler.second->updateLocalMap();
    handler.second->computeValues();
  }
}

void GlobalDensityMap::writeMaps() {
  fileWriter->writeData();

  // write decentralized maps
  for (auto &handler : dezentralMaps) {
    handler.second->writeMap();
  }
}

}  // namespace crownet
