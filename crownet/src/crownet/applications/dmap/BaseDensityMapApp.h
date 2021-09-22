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

#include "inet/common/InitStages.h"
#include "artery/application/Middleware.h"
#include "artery/utility/IdentityRegistry.h"

#include "crownet/applications/common/AppFsm.h"
#include "crownet/applications/common/BaseApp.h"
#include "crownet/common/IDensityMapHandler.h"
#include "crownet/common/converter/OsgCoordConverter.h"
#include "crownet/common/util/FileWriter.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"
#include "crownet/dcd/generic/CellVisitors.h"
#include "crownet/dcd/regularGrid/RegularCell.h"

using namespace omnetpp;
using namespace inet;


namespace crownet {
class BaseDensityMapApp : public BaseApp,
                          public IDensityMapHandler<RegularDcdMap>,
                          public omnetpp::cListener {
public:
    virtual ~BaseDensityMapApp();
    BaseDensityMapApp(){};


protected:
 // cSimpleModule
 virtual int numInitStages() const override { return NUM_INIT_STAGES; }
 virtual void initialize(int stage) override;
 using omnetpp::cIListener::finish;  // [-Woverloaded-virtual]
 virtual void finish() override;

 // cListener
 void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj,
                    cObject *details) override;

 virtual FsmState handleDataArrived(Packet *packet) override;

 //
 virtual Packet *createPacket() override;

 // FSM
 virtual FsmState fsmSetup(cMessage *msg) override;
 virtual FsmState fsmAppMain(cMessage *msg) override;

 // App logic
 virtual void initDcdMap();
 virtual void initWriter();
 virtual bool mergeReceivedMap(Packet *packet);

 // IDensityMapHandler
 virtual void updateLocalMap() override;
 virtual void computeValues() override;
 virtual void writeMap() override;
 virtual std::shared_ptr<RegularDcdMap> getMap() override;
 virtual void setDistanceProvider(std::shared_ptr<GridCellDistance> distProvider) override;
 virtual void setCoordinateConverter(std::shared_ptr<OsgCoordinateConverter> converter) override;

protected:

 std::shared_ptr<OsgCoordinateConverter> converter;
 std::shared_ptr<RegularDcdMap> dcdMap;
 std::unique_ptr<FileWriter> fileWriter;
 std::shared_ptr<TimestampedGetEntryVisitor<RegularCell>> valueVisitor;
 std::shared_ptr<GridCellDistance> distProvider;
 simtime_t lastUpdate = -1.0;
 std::string mapType;
 std::string mapTypeLog;

 RegularDcdMapWatcher* watcher;
 cMessage *localMapTimer;
 cPar *localMapUpdateInterval;

};

} // namesapce crownet
