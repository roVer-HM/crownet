/*
 * ControlHandler.h
 *
 *  Created on: May 27, 2021
 *      Author: vm-sts
 */

#ifndef CROWNET_CONTROL_CONTROLHANDLER_H_
#define CROWNET_CONTROL_CONTROLHANDLER_H_


namespace crownet {

struct ForwardCmd {
    int cmdId;
    int varId;
    std::string objectIdentifer;
    int offset;
    int cmdLength;
    int payloadOffset;
    int payloadLength;
};

struct ControlCmd {
    std::string sendingNode;
    std::string model;
    std::string message;
};


struct DensityMapCmd {
    std::string nodeId;
};

class ControlHandler {
public:
    virtual ~ControlHandler() = default;
    virtual void handleActionCommand(const ControlCmd& cmd) = 0;
    virtual std::vector<std::string> handleDensityMapCommand(const DensityMapCmd& cmd) = 0;

    //todo: (CM) add handle method for sensor command here

};


};

#endif /* CROWNET_CONTROL_CONTROLHANDLER_H_ */
