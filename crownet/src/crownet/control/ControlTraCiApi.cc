/*
 * ControlTraCiApi.cc
 *
 *  Created on: Feb 26, 2021
 *      Author: vm-sts
 */

#include <omnetpp/cexpression.h>
#include "ControlTraCiApi.h"

using namespace libsumo;

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
    cmd.cmdId = inMsg.readUnsignedByte(); // cmdId -> executeControlCommand
    cmd.varId = inMsg.readUnsignedByte(); // varId -> what Control to execute
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

    createCommand(traci::constants::CMD_CONTROL, CMD_SIMSTEP, objId, &content);
    // call controller and set next time
    return handleControlLoop();
}

double ControlTraCiApi::handleInit(double simtime){

    std::string objId = "";
    tcpip::Storage content;
    content.writeByte(TYPE_COMPOUND);
    content.writeInt(1);
    content.writeByte(TYPE_DOUBLE);
    content.writeDouble(simtime);

    createCommand(traci::constants::CMD_CONTROL, traci::constants::VAR_INIT, objId, &content);
    return handleControlLoop();
}


double ControlTraCiApi::handleControlLoop(){
    // send  command to controller
    mySocket->sendExact(myOutput);
    myInput.reset();
    bool running = true;
    double nextControlUpdateAt = -1.0; // -1.0 means next simStep

    while(running){
        // receive myInput and decided what to do
        // replies must be CMD_CONTROL commands. no result State provided.
        mySocket->receiveExact(myInput);
        ForwardCmd result = parseCtrlCmd(myInput);

        if (result.cmdId != traci::constants::CMD_CONTROL){
            throw omnetpp::cRuntimeError("#Error: expected CMD_CONTROL");
        }

        if (result.varId == traci::constants::VAR_Step || result.varId == traci::constants::VAR_INIT){
            // Acknowledgment of simStep from controller [simAck]
            // read next call time
            if (myInput.readUnsignedByte() != TYPE_DOUBLE){
                throw omnetpp::cRuntimeError("#Error: expected TYPE_DOUBLE");
            }
            nextControlUpdateAt = myInput.readDouble();

            // stop receiving mode. (Other side is waiting for next 'sendExact' call)
            running = false;
        } else if (result.varId == traci::constants::VAR_FORWARD){

            if (result.objectIdentifer == traci::constants::SIMULATOR_VADERE){

                  // extract payload and forward
                  tcpip::Storage forwardCmd;
                  tcpip::Storage forwardResponse;

                  forwardCmd.writeStorage(myInput, result.payloadLength);
                  traciForwarder->forward(forwardCmd, forwardResponse);

                  // send response from mobilityProvider to controller
                  mySocket->sendExact(forwardResponse);

              } else if (result.objectIdentifer == traci::constants::SIMULATOR_OPP){
                  // extract command and execute
                  tcpip::Storage res = handleControllerOppRequest(result);
                  // send response to controller
                  mySocket->sendExact(res);
              } else {
                  throw omnetpp::cRuntimeError("#Error: expected 'V' or 'O' as objectID to select simulator");
              }

            // keep receiving commands
            running = true;
        }
        else {
            throw omnetpp::cRuntimeError("#Error: expected VAR_Step or VAR_FORWARD");
        }
    }

    return nextControlUpdateAt;
}

tcpip::Storage ControlTraCiApi::handleControllerOppRequest(ForwardCmd& ctrlCmd){
    // extract payload and forward
     tcpip::Storage cmd;
     ControlCmd controlCmd;
     DensityMapCmd densityCmd;
     cmd.writeStorage(myInput, ctrlCmd.payloadLength);
     int cmdLength = cmd.readCmdLength();
     int cmdId = cmd.readUnsignedByte();
     int varId = cmd.readUnsignedByte(); // varId = 32, das was von python geschickt wurde
     std::string objectIdentifer = cmd.readString();

     //todo: (CM) check cmdId to separate controlCmd and Sensor query


     // compound object
     int type = cmd.readUnsignedByte();
     if (type != TYPE_COMPOUND){
         throw omnetpp::cRuntimeError("expected compound object got");
     }
     int cmpSize = cmd.readInt();
     // todo if else if
     // if cmpSize == 1 schauen ob .ID für Dichtekarte gesetzt ist
     if(cmpSize == 1){ // varId == 34
         type = cmd.readUnsignedByte();
         densityCmd.nodeId = cmd.readString();
         std::vector<std::string> payload =
                 this->controlHandler->handleDensityMapCommand(densityCmd);
         tcpip::Storage response;
         this->createPayloadResponse(response, cmdId, RTYPE_OK, "", payload);
         return response;
     } else if (cmpSize == 3){ // varId == 32
         type = cmd.readUnsignedByte();
         controlCmd.sendingNode = cmd.readString();
         type = cmd.readUnsignedByte();
         controlCmd.model = cmd.readString();
         type = cmd.readUnsignedByte();
         controlCmd.message = cmd.readString();
         this->controlHandler->handleActionCommand(controlCmd);

     }
     else {
         throw omnetpp::cRuntimeError("expected 3 items in compound object");
     }

     tcpip::Storage response;
     this->createResponse(response, cmdId, RTYPE_OK, "");
     return response;
}




} /* namespace crownet */
