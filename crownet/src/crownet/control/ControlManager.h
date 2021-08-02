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
#include <memory.h>

#include <traci/Listener.h>
#include "crownet/control/ControlTraCiApi.h"
#include "crownet/control/ControlHandler.h"
#include "crownet/common/GlobalDensityMap.h"
#include "inet/common/InitStages.h"

using namespace omnetpp;
using namespace inet;

namespace crownet {
/**
 * TODO - Generated class
 */
class ControlManager : public cSimpleModule, public ControlHandler,  public traci::Listener
{
public:
    virtual ~ControlManager();

  protected:
    virtual void initialize(int stage) override ;
    virtual int numInitStages() const  override {return NUM_INIT_STAGES;}
    virtual void handleMessage(cMessage *msg) override;

    // implement ControlHander interface
    virtual void handleActionCommand(const ControlCmd& cmd) override;
    virtual std::vector<std::string> handleDensityMapCommand(const DensityMapCmd& cmd) override;


    using omnetpp::cIListener::finish;  // [-Woverloaded-virtual]
    void finish() override;

  private:
    void traciInit() override;
    void traciStep() override;
    void traciClose() override;

  private:
    std::shared_ptr<ControlTraCiApi> api;
    GlobalDensityMap* globalMap;
    simtime_t nextTime = simtime_t::ZERO;
    std::string controlGate;
};

}
