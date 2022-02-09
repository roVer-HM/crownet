/*
 * VadereApi.cc
 *
 *  Created on: Aug 6, 2020
 *      Author: sts
 */

#include "crownet/artery/traci/VadereApi.h"

using namespace crownet::constants;
using namespace libsumo;

namespace crownet {

TraCIGeoPosition VadereApi::convertGeo(const TraCIPosition& pos) const {
  if (converter == nullptr) {
    return API::convertGeo(pos);
  } else {
    return converter->convertToGeoTraCi(pos);
  }
}

TraCIPosition VadereApi::convert2D(const TraCIGeoPosition& pos) const {
  if (converter != nullptr) {
    return API::convert2D(pos);
  } else {
    return converter->convertToCartTraCIPosition(pos);
  }
}

void VadereApi::forward(tcpip::Storage& msgIn,  tcpip::Storage& msgResponse){
    mySocket->sendExact(msgIn);
    mySocket->receiveExact(msgResponse);
}

VadereApi::VadereApi() : API(), v_simulation(*this), v_person(*this) {
  // override domain response
  myDomains[RESPONSE_SUBSCRIBE_SIM_VARIABLE] = &v_simulation;
  myDomains[RESPONSE_SUBSCRIBE_PERSON_VARIABLE] = &v_person;
}

// ---------------------------------------------------------------------------
// // TraCIAPI::VaderePersonScope-methods
//  ---------------------------------------------------------------------------

// not needed implementen in TraCIAPI
//std::vector<std::string> VadereApi::VaderePersonScope::getIDList() const {
//  return getStringVector(CMD_GET_PERSON_VARIABLE, TRACI_ID_LIST, "");
//}

// not needed implementen in TraCIAPI
//int VadereApi::VaderePersonScope::getIDCount() const {
//  return getInt(CMD_GET_PERSON_VARIABLE, ID_COUNT, "");
//}

double VadereApi::VaderePersonScope::getSpeed(
    const std::string& personID) const {
  return getDouble(VAR_SPEED, personID);
}

libsumo::TraCIPosition VadereApi::VaderePersonScope::getPosition(
    const std::string& personID) const {
  return getPos(VAR_POSITION, personID);
}

std::string VadereApi::VaderePersonScope::getTypeID(
    const std::string& personID) const {
  return getString(VAR_TYPE, personID);
}

double VadereApi::VaderePersonScope::getAngle(
    const std::string& personID) const {
  return getDouble(VAR_ANGLE, personID);
}

libsumo::TraCIColor VadereApi::VaderePersonScope::getColor(
    const std::string& personID) const {
  return getCol(VAR_COLOR, personID);
}

std::vector<std::string> VadereApi::VaderePersonScope::getTargetList(
    const std::string& personID) const {
  return getStringVector(VAR_TARGET_LIST, personID);
}

void VadereApi::VaderePersonScope::setSpeed(const std::string& personID,
                                            double speed) const {
  tcpip::Storage content;
  content.writeUnsignedByte(TYPE_DOUBLE);
  content.writeDouble(speed);
  createSetCommand(VAR_SPEED, personID, &content);
//  send_commandSetValue(CMD_SET_PERSON_VARIABLE, VAR_SPEED, personID, content);
//  tcpip::Storage inMsg;
//  check_resultState(inMsg, CMD_SET_PERSON_VARIABLE);
  processSet();
}

void VadereApi::VaderePersonScope::setType(const std::string& personID,
                                           const std::string& typeID) const {
  tcpip::Storage content;
  content.writeUnsignedByte(TYPE_STRING);
  content.writeString(typeID);
  createSetCommand(VAR_TYPE, personID, &content);
//  send_commandSetValue(CMD_SET_PERSON_VARIABLE, VAR_TYPE, personID, content);
//  tcpip::Storage inMsg;
//  check_resultState(inMsg, CMD_SET_PERSON_VARIABLE);
  processSet();
}

void VadereApi::VaderePersonScope::setLength(const std::string& personID,
                                             double length) const {
  tcpip::Storage content;
  content.writeUnsignedByte(TYPE_DOUBLE);
  content.writeDouble(length);
  createSetCommand(VAR_LENGTH, personID, &content);
//  send_commandSetValue(CMD_SET_PERSON_VARIABLE, VAR_LENGTH, personID, content);
//  tcpip::Storage inMsg;
//  check_resultState(inMsg, CMD_SET_PERSON_VARIABLE);
  processSet();
}

void VadereApi::VaderePersonScope::setWidth(const std::string& personID,
                                            double width) const {
  tcpip::Storage content;
  content.writeUnsignedByte(TYPE_DOUBLE);
  content.writeDouble(width);
  createSetCommand(VAR_WIDTH, personID, &content);
//  send_commandSetValue(CMD_SET_PERSON_VARIABLE, VAR_WIDTH, personID, content);
//  tcpip::Storage inMsg;
//  check_resultState(inMsg, CMD_SET_PERSON_VARIABLE);
  processSet();
}

void VadereApi::VaderePersonScope::setHeight(const std::string& personID,
                                             double height) const {
  tcpip::Storage content;
  content.writeUnsignedByte(TYPE_DOUBLE);
  content.writeDouble(height);
  createSetCommand(VAR_HEIGHT, personID, &content);
//  send_commandSetValue(CMD_SET_PERSON_VARIABLE, VAR_HEIGHT, personID, content);
//  tcpip::Storage inMsg;
//  check_resultState(inMsg, CMD_SET_PERSON_VARIABLE);
  processSet();
}

void VadereApi::VaderePersonScope::setColor(
    const std::string& personID, const libsumo::TraCIColor& c) const {
  tcpip::Storage content;
  content.writeUnsignedByte(TYPE_COLOR);
  content.writeUnsignedByte(c.r);
  content.writeUnsignedByte(c.g);
  content.writeUnsignedByte(c.b);
  content.writeUnsignedByte(c.a);
  createSetCommand(VAR_COLOR, personID, &content);
//  send_commandSetValue(CMD_SET_PERSON_VARIABLE, VAR_COLOR, personID, content);
//  tcpip::Storage inMsg;
//  check_resultState(inMsg, CMD_SET_PERSON_VARIABLE);
  processSet();
}

void VadereApi::VaderePersonScope::setTargetList(
    const std::string& personID, std::vector<std::string> targets) const {
  tcpip::Storage content;
  content.writeUnsignedByte(TYPE_STRINGLIST);
  content.writeStringList(targets);
  createSetCommand(VAR_TARGET_LIST, personID, &content);
//  send_commandSetValue(CMD_SET_PERSON_VARIABLE, VAR_TARGET_LIST, personID,
//                       content);
//  tcpip::Storage inMsg;
//  check_resultState(inMsg, CMD_SET_PERSON_VARIABLE);
  processSet();
}

void VadereApi::VaderePersonScope::setInformation(
    const std::string& personID, double start_at, double obsolete_at,
    std::string information) const {
  tcpip::Storage content;
  content.writeByte(TYPE_COMPOUND);
  content.writeInt(3);
  content.writeByte(TYPE_DOUBLE);
  content.writeDouble(start_at);
  content.writeByte(TYPE_DOUBLE);
  content.writeDouble(obsolete_at);
  content.writeByte(TYPE_STRING);
  content.writeString(information);
  createSetCommand(VAR_PERSON_STIMULUS, personID, &content);
//  send_commandSetValue(CMD_SET_PERSON_VARIABLE, VAR_PERSON_STIMULUS, personID,
//                       content);
//  tcpip::Storage inMsg;
//  check_resultState(inMsg, CMD_SET_PERSON_VARIABLE);
  processSet();
}



// ---------------------------------------------------------------------------
// VadereApi::VaderSimulationScope-methods
// ---------------------------------------------------------------------------
int VadereApi::VaderSimulationScope::getCurrentTime() const {
//  return myParent.getInt(CMD_GET_SIM_VARIABLE, VAR_TIME_STEP, "");
    return getInt(VAR_TIME_STEP, "");
}

double VadereApi::VaderSimulationScope::getTime() const {
//  return myParent.getDouble(CMD_GET_SIM_VARIABLE, VAR_TIME, "");
    return getDouble(VAR_TIME, "");
}

int VadereApi::VaderSimulationScope::getLoadedNumber() const {
//  return (int)myParent.getInt(CMD_GET_SIM_VARIABLE, VAR_LOADED_VEHICLES_NUMBER,
//                              "");
    return (int)getInt(VAR_LOADED_VEHICLES_NUMBER,"");
}

std::vector<std::string> VadereApi::VaderSimulationScope::getLoadedIDList()
    const {
//  return myParent.getStringVector(CMD_GET_SIM_VARIABLE, VAR_LOADED_VEHICLES_IDS,
//                                  "");
    return getStringVector(VAR_LOADED_VEHICLES_IDS,"");
}

int VadereApi::VaderSimulationScope::getDepartedNumber() const {
//  return (int)myParent.getInt(CMD_GET_SIM_VARIABLE,
//                              VAR_DEPARTED_VEHICLES_NUMBER, "");
    return (int)getInt(VAR_DEPARTED_VEHICLES_NUMBER, "");
}

std::vector<std::string> VadereApi::VaderSimulationScope::getDepartedIDList()
    const {
//  return myParent.getStringVector(CMD_GET_SIM_VARIABLE,
//                                  VAR_DEPARTED_VEHICLES_IDS, "");
    return getStringVector(VAR_DEPARTED_VEHICLES_IDS, "");
}

int VadereApi::VaderSimulationScope::getArrivedNumber() const {
//  return (int)myParent.getInt(CMD_GET_SIM_VARIABLE, VAR_ARRIVED_VEHICLES_NUMBER,
//                              "");
    return (int)getInt(VAR_ARRIVED_VEHICLES_NUMBER,"");
}

std::vector<std::string> VadereApi::VaderSimulationScope::getArrivedIDList()
    const {
//  return myParent.getStringVector(CMD_GET_SIM_VARIABLE,
//                                  VAR_ARRIVED_VEHICLES_IDS, "");
    return getStringVector(VAR_ARRIVED_VEHICLES_IDS, "");
}

double VadereApi::VaderSimulationScope::getDeltaT() const {
//  return myParent.getDouble(CMD_GET_SIM_VARIABLE, VAR_DELTA_T, "");
    return getDouble(VAR_DELTA_T, "");
}

libsumo::TraCIPositionVector VadereApi::VaderSimulationScope::getNetBoundary()
    const {
//  return myParent.getPolygon(CMD_GET_SIM_VARIABLE, VAR_NET_BOUNDING_BOX, "");
    return getPolygon(VAR_NET_BOUNDING_BOX, "");
}

double VadereApi::VaderSimulationScope::getDistance2D(double x1, double y1,
                                                      double x2, double y2,
                                                      bool isGeo,
                                                      bool isDriving) {
  tcpip::Storage content;
  content.writeByte(TYPE_COMPOUND);
  content.writeInt(3);
  content.writeByte(isGeo ? POSITION_LON_LAT : POSITION_2D);
  content.writeDouble(x1);
  content.writeDouble(y1);
  content.writeByte(isGeo ? POSITION_LON_LAT : POSITION_2D);
  content.writeDouble(x2);
  content.writeDouble(y2);
  content.writeByte(isDriving ? REQUEST_DRIVINGDIST : REQUEST_AIRDIST);
  createGetCommand(DISTANCE_REQUEST, "", &content);
//  send_commandGetVariable(CMD_GET_SIM_VARIABLE, DISTANCE_REQUEST, "", &content);
//  tcpip::Storage inMsg;
//  processGET(inMsg, CMD_GET_SIM_VARIABLE, TYPE_DOUBLE);
  processGet(TYPE_DOUBLE);
  return getInput().readDouble();

}

std::string VadereApi::VaderSimulationScope::getScenarioHash(
    const std::string& scenario) const {
    tcpip::Storage content;
    content.writeByte(TYPE_STRING);
    content.writeString(scenario);
    createGetCommand(crownet::constants::VAR_CACHE_HASH, "", &content);
//  send_commandGetVariableExtLenghtField(
//      myCmdGetID, crownet::constants::VAR_CACHE_HASH, "", &content);
//  tcpip::Storage inMsg;
//  processGET(inMsg, myCmdGetID, TYPE_STRING);
//  return inMsg.readString();
   processGet(TYPE_STRING);
   return getInput().readString();
}

CoordRef VadereApi::VaderSimulationScope::getCoordRef() const {
    createGetCommand(constants::VAR_COORD_REF, "");
    processGet(TYPE_COMPOUND);
//  send_commandGetVariable(myCmdGetID, constants::VAR_COORD_REF, "");
  tcpip::Storage& inMsg = getInput();
//  processGET(inMsg, myCmdGetID, TYPE_COMPOUND, true);
  CoordRef ref;

  inMsg.readInt();           // number of vars (3)
  inMsg.readUnsignedByte();  // type

  ref.epsg_code = inMsg.readString();
  inMsg.readUnsignedByte();
  ref.offset.x = inMsg.readDouble();
  inMsg.readUnsignedByte();
  ref.offset.y = inMsg.readDouble();
  ref.offset.z = 0.0;
  return ref;
}

void VadereApi::VaderSimulationScope::sendSimulationConfig(
    const vadere::SimCfg& cfg) const {
  tcpip::Storage content;
  content.writeByte(TYPE_COMPOUND);
  content.writeInt(10);
  content.writeByte(TYPE_STRING);
  content.writeString(cfg.oppConfigName);
  content.writeByte(TYPE_STRING);
  content.writeString(cfg.oppExperiment);
  content.writeByte(TYPE_STRING);
  content.writeString(cfg.oppDateTime);
  content.writeByte(TYPE_STRING);
  content.writeString(cfg.oppResultRootDir);
  content.writeByte(TYPE_STRING);
  content.writeString(cfg.oppIterationVariables);
  content.writeByte(TYPE_STRING);
  content.writeString(cfg.oppRepetition);
  content.writeByte(TYPE_STRING);
  content.writeString(cfg.oppOutputScalarFile);
  content.writeByte(TYPE_STRING);
  content.writeString(cfg.oppOutputVecFile);
  content.writeByte(TYPE_INTEGER);
  content.writeInt(cfg.seed);
  content.writeByte(TYPE_UBYTE);
  content.writeUnsignedByte(cfg.useVadereSeed);

  createSetCommand(crownet::constants::VAR_SIM_CONFIG, "", &content);
//  send_commandSetValueExtLenghtField(
//      myCmdSetID, crownet::constants::VAR_SIM_CONFIG, "", content);
//  tcpip::Storage inMsg;
//  check_resultState(inMsg, myCmdSetID);
  processSet();
}

void VadereApi::VaderSimulationScope::send_control(const std::string& personID, std::string model, std::string metadata) const
{
    tcpip::Storage content;
    content.writeByte(TYPE_COMPOUND);
    content.writeInt(3);
    content.writeByte(TYPE_STRING);
    content.writeString(personID); // the current nodeId
    content.writeByte(TYPE_STRING);
    content.writeString(model); // model name to use
    content.writeByte(TYPE_STRING);
    content.writeString(metadata); // json to configure given model

    createSetCommand(crownet::constants::VAR_EXTERNAL_INPUT, "", &content);
//    send_commandSetValueExtLenghtField(
//        myCmdSetID, crownet::constants::VAR_EXTERNAL_INPUT, "", content);
//    tcpip::Storage inMsg;
//    check_resultState(inMsg, myCmdSetID);
    processSet();

}
//todo: (CM) add method to translate a DCD map into a traci packet. (implement here)


} /* namespace crownet */
