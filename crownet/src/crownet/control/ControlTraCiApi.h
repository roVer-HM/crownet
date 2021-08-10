/*
 * ControlTraCiApi.h
 *
 *  Created on: Feb 26, 2021
 *      Author: vm-sts
 */

#pragma once

#include <memory.h>

#include <traci/sumo/utils/traci/TraCIAPI.h>
#include "crownet/artery/traci/TraCiForwarder.h"
#include "crownet/control/ControlHandler.h"

namespace traci {
namespace constants {

// command: simulation step
// constexpr ubyte CMD_CONTROL = 0x0d;
constexpr int CMD_CONTROL = 0x06;
constexpr int RESPONSE_CMD_CONTROL = 0x1d;

constexpr int VAR_FORWARD = 0xff;
constexpr int VAR_INIT = 0x00;
constexpr int VAR_Step = 0x02;
constexpr int VAR_DENSITY_MAP = 0x22;
constexpr int VAR_EXTERNAL_INPUT_INIT = 0x21;
constexpr int VAR_EXTERNAL_INPUT = 0x20;

constexpr char SIMULATOR_VADERE[] = "V";
constexpr char SIMULATOR_OPP[] = "O";


}
}

namespace crownet {

class ControlTraCiApi: public TraCIAPI {
public:
    ControlTraCiApi();
    virtual ~ControlTraCiApi();


    virtual double handleSimStep(double simtime);
    virtual double handleInit(double simtime);
    void setTraCiForwarder(std::shared_ptr<TraCiForwarder> traciForwarder);
    void setControlHandler(ControlHandler* controlHandler);

protected:
    virtual double handleControlLoop();
    virtual tcpip::Storage handleControllerOppRequest(ForwardCmd& ctrlCmd);

    ForwardCmd parseCtrlCmd(tcpip::Storage& inMsg);
    libsumo::TraCIResult parseResult(tcpip::Storage& inMsg);

private:
    void checkCompound(tcpip::Storage& cmd, int numElements);


    std::shared_ptr<TraCiForwarder> traciForwarder;
    ControlHandler* controlHandler;
};

} /* namespace crownet */

