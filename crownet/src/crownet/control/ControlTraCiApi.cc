/*
 * ControlTraCiApi.cc
 *
 *  Created on: Feb 26, 2021
 *      Author: vm-sts
 */

#include <omnetpp/cexpression.h>
#include "ControlTraCiApi.h"

namespace crownet {

ControlTraCiApi::ControlTraCiApi() {
    // TODO Auto-generated constructor stub

}

ControlTraCiApi::~ControlTraCiApi() {
    // TODO Auto-generated destructor stub
}

void ControlTraCiApi::setTraCiForwarder(std::shared_ptr<TraCiForwarder> traciForwarder){
    this->traciForwarder = traciForwarder;
}

void ControlTraCiApi::setControlHandler(ControlHandler* controlHandler){
    this->controlHandler = controlHandler;
}

ForwardCmd ControlTraCiApi::parseCtrlCmd(tcpip::Storage& inMsg){
    ForwardCmd cmd;
    cmd.offset = inMsg.position();
    cmd.cmdLength = inMsg.readCmdLength();
    cmd.cmdId = inMsg.readUnsignedByte();
    cmd.varId = inMsg.readUnsignedByte();
    cmd.objectIdentifer = inMsg.readString();
    cmd.payloadOffset = inMsg.position();
    cmd.payloadLength = (int)inMsg.size() - cmd.payloadOffset;
    return cmd;
}

double ControlTraCiApi::handleSimStep(double simtime){

    std::string objId = "";
    tcpip::Storage content;
    content.writeByte(TYPE_COMPOUND);
    content.writeInt(1);
    content.writeByte(TYPE_DOUBLE);
    content.writeDouble(simtime);

    //todo directly pull vadere subscription here to reduce roundtrip?
    tcpip::Storage outMsg = build_command(CMD_CONTROL, CMD_SIMSTEP, objId, content);
    // call controller and set next time
    return handleControlCmd(outMsg);
}

double ControlTraCiApi::handleInit(double simtime){

    std::string objId = "";
    tcpip::Storage content;
    content.writeByte(TYPE_COMPOUND);
    content.writeInt(1);
    content.writeByte(TYPE_DOUBLE);
    content.writeDouble(simtime);

    tcpip::Storage outMsg = build_command(CMD_CONTROL, VAR_INIT, objId, content);
    // call controller and set next time
    return handleControlCmd(outMsg);
}


double ControlTraCiApi::handleControlCmd(tcpip::Storage& ctrlCmd){
    // send  command to controller
    mySocket->sendExact(ctrlCmd);
    bool running = true;
    double nextControlUpdateAt = -1.0; // -1.0 means next simStep

    while(running){
        // receive replay and decided what to do
        // replies must be CMD_CONTROL commands. no result State provided.
        tcpip::Storage reply;
        mySocket->receiveExact(reply);
        ForwardCmd result = parseCtrlCmd(reply);

        if (result.cmdId != CMD_CONTROL){
            throw omnetpp::cRuntimeError("#Error: expected CMD_CONTROL");
        }

        if (result.varId == VAR_Step || result.varId == VAR_INIT){
            // Acknowledgment of simStep from controller [simAck]
            // read next call time
            if (reply.readUnsignedByte() != TYPE_DOUBLE){
                throw omnetpp::cRuntimeError("#Error: expected TYPE_DOUBLE");
            }
            nextControlUpdateAt = reply.readDouble();

            // stop receiving mode. (Other side is waiting for next 'sendExact' call)
            running = false;
        } else if (result.varId == VAR_FORWARD){

            if (result.objectIdentifer == SIMULATOR_VADERE){

                  // extract payload and forward
                  tcpip::Storage forwardCmd;
                  tcpip::Storage forwardResponse;

                  forwardCmd.writeStorage(reply, result.payloadLength);
                  traciForwarder->forward(forwardCmd, forwardResponse);

                  // send response from mobilityProvider to controller
                  mySocket->sendExact(forwardResponse);

              } else if (result.objectIdentifer == SIMULATOR_OPP){
                  // extract command and execute
                  tcpip::Storage res = handleControllerOppRequest(reply, result);
                  // send response to controller
                  mySocket->sendExact(res);
              } else {
                  throw omnetpp::cRuntimeError("#Error: expected 'V' or 'O' as objectID to select simulator");
              }

            // keep receiving commands
            running = true;
        } else {
            throw omnetpp::cRuntimeError("#Error: expected VAR_Step or VAR_FORWARD");
        }
    }

    return nextControlUpdateAt;
}

tcpip::Storage ControlTraCiApi::handleControllerOppRequest(tcpip::Storage& msgIn, ForwardCmd& ctrlCmd){
    // extract payload and forward
     tcpip::Storage cmd;
     cmd.writeStorage(msgIn, ctrlCmd.payloadLength);
     int cmdLength = cmd.readCmdLength();
     int cmdId = cmd.readUnsignedByte();
     int varId = cmd.readUnsignedByte();
     std::string objectIdentifer = cmd.readString();

     //todo: (CM) check cmdId to separate controlCmd and Sensor query

     ControlCmd controlCmd;
     // compound object
     int type = cmd.readUnsignedByte();
     if (type != TYPE_COMPOUND){
         throw omnetpp::cRuntimeError("expected compound object got");
     }
     int cmpSize = cmd.readInt();
     if (cmpSize != 3) {
         throw omnetpp::cRuntimeError("expected 3 items in compound object");
     }
     type = cmd.readUnsignedByte();
     controlCmd.sendingNode = cmd.readString();
     type = cmd.readUnsignedByte();
     controlCmd.model = cmd.readString();
     type = cmd.readUnsignedByte();
     controlCmd.message = cmd.readString();


     this->controlHandler->handleCommand(controlCmd);

     tcpip::Storage response;
     this->createResponse(response, cmdId, RTYPE_OK, "");
     return response;
}




} /* namespace crownet */
