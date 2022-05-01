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

void ControlTraCiApi::checkCompound(tcpip::Storage& cmd, int numElements){
    int type = cmd.readUnsignedByte();
    if (type != TYPE_COMPOUND){
        throw omnetpp::cRuntimeError("expected compound object (TYPE_COMPOUND: 0x%02x) got 0x%02x", TYPE_COMPOUND, type);
    }
    int cmpSize = cmd.readInt();
    if (cmpSize != numElements){
        throw omnetpp::cRuntimeError("expected %i items in compound object got %i", numElements, cmpSize);
    }

}

tcpip::Storage ControlTraCiApi::handleControllerOppRequest(ForwardCmd& ctrlCmd){
    // extract payload and forward
     tcpip::Storage cmd;
     cmd.writeStorage(myInput, ctrlCmd.payloadLength);
     int cmdId = cmd.readUnsignedByte();
     int varId = cmd.readUnsignedByte(); // varId = 32, das was von python geschickt wurde
     std::string objectIdentifer = cmd.readString();


     tcpip::Storage response;

     if(varId == traci::constants::VAR_DENSITY_MAP){ // varId == 34
         // compound object
         checkCompound(cmd, 1);

         DensityMapCmd densityCmd;
         cmd.readUnsignedByte(); // string
         densityCmd.nodeId = cmd.readString();

         std::vector<double> payload =
                 this->controlHandler->handleDensityMapCommand(densityCmd);
         this->createResponse(response, cmdId, RTYPE_OK, ""); // assume all is ok

         tcpip::Storage cmdData;
         cmdData.writeByte(TYPE_DOUBLELIST);
         cmdData.writeDoubleList(payload);
         // create result command for given cmdId. Use ResultComandId in this
         // command where the result commandID equals cmdID+0x10
         // createCommand(...) will create the command in this->myOutput
         this->createCommand(cmdId+0x10, varId, objectIdentifer, &cmdData);
         // append command output to response object and reset this->myOutput
         response.writeStorage(myOutput);
         myOutput.reset();

     } else if (varId == traci::constants::VAR_EXTERNAL_INPUT){ // varId == 32
         checkCompound(cmd, 3);

         ControlCmd controlCmd;
         cmd.readUnsignedByte(); // string
         controlCmd.sendingNode = cmd.readString();
         cmd.readUnsignedByte(); // string
         controlCmd.model = cmd.readString();
         cmd.readUnsignedByte(); // string
         controlCmd.message = cmd.readString();
         this->controlHandler->handleActionCommand(controlCmd);
         this->createResponse(response, cmdId, RTYPE_OK, ""); // assume all is ok

     }
     else {
         throw omnetpp::cRuntimeError("expected 3 items in compound object");
     }


     return response;
}




} /* namespace crownet */
