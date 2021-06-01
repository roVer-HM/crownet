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
    int packetSize;
    std::string sendingNode;
    std::string model;
    std::string message;
};

class ControlHandler {
public:
    virtual ~ControlHandler() = default;
    virtual void handleCommand(const ControlCmd& cmd) = 0;

};


};

#endif /* CROWNET_CONTROL_CONTROLHANDLER_H_ */
