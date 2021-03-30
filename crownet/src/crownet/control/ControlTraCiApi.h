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

namespace traci {
namespace constants {

// command: simulation step
constexpr ubyte CMD_CONTROL = 0x0d;
constexpr ubyte RESPONSE_CMD_CONTROL = 0x1d;

constexpr ubyte VAR_FORWARD = 0xff;
constexpr ubyte VAR_INIT = 0x00;
constexpr ubyte VAR_Step = 0x02;

constexpr char SIMULATOR_VADERE[] = "V";
constexpr char SIMULATOR_OPP[] = "O";


}
}

namespace crownet {

struct ControlCmd {
    int cmdId;
    int varId;
    std::string objectIdentifer;
    int offset;
    int cmdLength;
    int payloadOffset;
    int payloadLength;
};


class ControlTraCiApi: public TraCIAPI {
public:
    ControlTraCiApi();
    virtual ~ControlTraCiApi();


    virtual double handleSimStep(double simtime);
    virtual double handleInit(double simtime);
    void setTraCiForwarder(std::shared_ptr<TraCiForwarder> traciForwarder);

protected:
    virtual double handleControlCmd(tcpip::Storage& ctrlCmd);

    virtual tcpip::Storage handleControllerOppRequest(tcpip::Storage& msgIn);

    ControlCmd parseCtrlCmd(tcpip::Storage& inMsg);
    TraCiResult parseResult(tcpip::Storage& inMsg);

    std::shared_ptr<TraCiForwarder> traciForwarder;
};

} /* namespace crownet */

