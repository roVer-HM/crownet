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

#pragma once

#include <omnetpp.h>
#include <omnetpp/clistener.h>
#include <omnetpp/csimplemodule.h>
#include <memory>

#include "inet/common/InitStages.h"
#include "crownet/common/IDensityMapHandler.h"
#include "crownet/common/converter/OsgCoordConverter.h"
#include "crownet/common/util/FileWriter.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"
#include "traci/NodeManager.h"

using namespace omnetpp;
using namespace inet;

namespace crownet {

class GlobalDensityMap : public omnetpp::cSimpleModule,
                         public omnetpp::cListener,
                         public traci::ITraciNodeVisitor {
 public:
  //  using Grid = RegularGridMap<std::string>;
  using GridHandler = IDensityMapHandler<RegularDcdMap>;
  using gridMap_t = std::map<RegularDcdMap::node_key_t, GridHandler *>;

  static const omnetpp::simsignal_t registerMap;
  static const omnetpp::simsignal_t removeMap;

  virtual ~GlobalDensityMap();

  // cSimpleModule
  virtual int numInitStages() const override { return NUM_INIT_STAGES; }
  virtual void initialize(int stage) override;

  // cListner
  virtual void initialize() override;
  using omnetpp::cIListener::finish;  // [-Woverloaded-virtual]
  virtual void finish() override;
  virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t,
                             omnetpp::cObject *, omnetpp::cObject *) override;
  virtual void receiveSignal(cComponent *source, simsignal_t signalID,
                             const SimTime &t, cObject *details) override;

  // ITraciNodeVisitor
  virtual void visitNode(const std::string &traciNodeId,
                         omnetpp::cModule *mod) override;

 protected:
  virtual void handleMessage(cMessage *msg) override;

  void updateMaps();
  void writeMaps();

 protected:
  cMessage *updateTimer = nullptr;
  simtime_t updateInterval;
  simtime_t lastUpdate;

  traci::NodeManager *nodeManager = nullptr;
  std::shared_ptr<OsgCoordinateConverter> converter;
  std::shared_ptr<RegularDcdMap> dcdMapGlobal;
  std::shared_ptr<GridCellDistance> distProvider;
  gridMap_t dezentralMaps;
  std::string m_mobilityModule;
  std::unique_ptr<FileWriter> fileWriter;
};

}  // namespace crownet
