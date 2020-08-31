/*
 * VadereApi.cc
 *
 *  Created on: Aug 6, 2020
 *      Author: sts
 */

#include "rover/artery/traci/VadereApi.h"

namespace rover {

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
    //    inet::Coord p = converter->convert2D(pos.longitude, pos.latitude);
    //    TraCIPosition out;
    //    out.x = p.x;
    //    out.y = p.y;
    //    out.z = p.z;
    //    return out;
  }
}

VadereApi::VadereApi() : API(), v_simulation(*this), v_person(*this) {
  // override domain response
  myDomains[RESPONSE_SUBSCRIBE_SIM_VARIABLE] = &v_simulation;
  myDomains[RESPONSE_SUBSCRIBE_PERSON_VARIABLE] = &v_person;
}

void VadereApi::sendFile(const vadere::VadereScenario& scenario) const {
  // todo: handle cache send? Currently cache will not be send

  tcpip::Storage outMsg;
  // command length (length[1+4=5] + cmdId[1] + strLengthField[4] +
  // strLenght[scenario.first] + strLengthField[4] + strLenght[scenario.second]

  int length = 5 + 1 + 4 + static_cast<int>(scenario.first.length()) + 4 +
               static_cast<int>(scenario.second.length());

  outMsg.writeUnsignedByte(
      0);  // first byte of extended length field must be zero
  outMsg.writeInt(length);

  outMsg.writeByte(rover::constants::CMD_FILE_SEND);
  //  outMsg.writeByte(TYPE_STRING);
  outMsg.writeString(scenario.first);
  //  outMsg.writeByte(TYPE_STRING);
  outMsg.writeString(scenario.second);

  mySocket->sendExact(outMsg);
  tcpip::Storage inMsg;
  check_resultState(inMsg, rover::constants::CMD_FILE_SEND);
}

// ---------------------------------------------------------------------------
// // TraCIAPI::VaderePersonScope-methods
//  ---------------------------------------------------------------------------

std::vector<std::string> VadereApi::VaderePersonScope::getIDList() const {
  return myParent.getStringVector(CMD_GET_PERSON_VARIABLE, ID_LIST, "");
}

int VadereApi::VaderePersonScope::getIDCount() const {
  return myParent.getInt(CMD_GET_PERSON_VARIABLE, ID_COUNT, "");
}

double VadereApi::VaderePersonScope::getSpeed(
    const std::string& personID) const {
  return myParent.getDouble(CMD_GET_PERSON_VARIABLE, VAR_SPEED, personID);
}

libsumo::TraCIPosition VadereApi::VaderePersonScope::getPosition(
    const std::string& personID) const {
  return myParent.getPosition(CMD_GET_PERSON_VARIABLE, VAR_POSITION, personID);
}

std::string VadereApi::VaderePersonScope::getTypeID(
    const std::string& personID) const {
  return myParent.getString(CMD_GET_PERSON_VARIABLE, VAR_TYPE, personID);
}

double VadereApi::VaderePersonScope::getAngle(
    const std::string& personID) const {
  return myParent.getDouble(CMD_GET_PERSON_VARIABLE, VAR_ANGLE, personID);
}

libsumo::TraCIColor VadereApi::VaderePersonScope::getColor(
    const std::string& personID) const {
  return myParent.getColor(CMD_GET_PERSON_VARIABLE, VAR_COLOR, personID);
}

/*
std::vector<std::string> VadereApi::VaderePersonScope::getTargetList() const {
  return myParent.getStringVector(CMD_GET_PERSON_VARIABLE, XXX_TARGET_LIST, "");
}
*/

void VadereApi::VaderePersonScope::setSpeed(const std::string& personID,
                                            double speed) const {
  tcpip::Storage content;
  content.writeUnsignedByte(TYPE_DOUBLE);
  content.writeDouble(speed);
  send_commandSetValue(CMD_SET_PERSON_VARIABLE, VAR_SPEED, personID, content);
  tcpip::Storage inMsg;
  check_resultState(inMsg, CMD_SET_PERSON_VARIABLE);
}

void VadereApi::VaderePersonScope::setType(const std::string& personID,
                                           const std::string& typeID) const {
  tcpip::Storage content;
  content.writeUnsignedByte(TYPE_STRING);
  content.writeString(typeID);
  send_commandSetValue(CMD_SET_PERSON_VARIABLE, VAR_TYPE, personID, content);
  tcpip::Storage inMsg;
  check_resultState(inMsg, CMD_SET_PERSON_VARIABLE);
}

void VadereApi::VaderePersonScope::setLength(const std::string& personID,
                                             double length) const {
  tcpip::Storage content;
  content.writeUnsignedByte(TYPE_DOUBLE);
  content.writeDouble(length);
  send_commandSetValue(CMD_SET_PERSON_VARIABLE, VAR_LENGTH, personID, content);
  tcpip::Storage inMsg;
  check_resultState(inMsg, CMD_SET_PERSON_VARIABLE);
}

void VadereApi::VaderePersonScope::setWidth(const std::string& personID,
                                            double width) const {
  tcpip::Storage content;
  content.writeUnsignedByte(TYPE_DOUBLE);
  content.writeDouble(width);
  send_commandSetValue(CMD_SET_PERSON_VARIABLE, VAR_WIDTH, personID, content);
  tcpip::Storage inMsg;
  check_resultState(inMsg, CMD_SET_PERSON_VARIABLE);
}

void VadereApi::VaderePersonScope::setHeight(const std::string& personID,
                                             double height) const {
  tcpip::Storage content;
  content.writeUnsignedByte(TYPE_DOUBLE);
  content.writeDouble(height);
  send_commandSetValue(CMD_SET_PERSON_VARIABLE, VAR_HEIGHT, personID, content);
  tcpip::Storage inMsg;
  check_resultState(inMsg, CMD_SET_PERSON_VARIABLE);
}

void VadereApi::VaderePersonScope::setColor(
    const std::string& personID, const libsumo::TraCIColor& c) const {
  tcpip::Storage content;
  content.writeUnsignedByte(TYPE_COLOR);
  content.writeUnsignedByte(c.r);
  content.writeUnsignedByte(c.g);
  content.writeUnsignedByte(c.b);
  content.writeUnsignedByte(c.a);
  send_commandSetValue(CMD_SET_PERSON_VARIABLE, VAR_COLOR, personID, content);
  tcpip::Storage inMsg;
  check_resultState(inMsg, CMD_SET_PERSON_VARIABLE);
}

// ---------------------------------------------------------------------------
// VadereApi::VaderSimulationScope-methods
// ---------------------------------------------------------------------------
int VadereApi::VaderSimulationScope::getCurrentTime() const {
  return myParent.getInt(CMD_GET_SIM_VARIABLE, VAR_TIME_STEP, "");
}

double VadereApi::VaderSimulationScope::getTime() const {
  return myParent.getDouble(CMD_GET_SIM_VARIABLE, VAR_TIME, "");
}

int VadereApi::VaderSimulationScope::getLoadedNumber() const {
  return (int)myParent.getInt(CMD_GET_SIM_VARIABLE, VAR_LOADED_VEHICLES_NUMBER,
                              "");
}

std::vector<std::string> VadereApi::VaderSimulationScope::getLoadedIDList()
    const {
  return myParent.getStringVector(CMD_GET_SIM_VARIABLE, VAR_LOADED_VEHICLES_IDS,
                                  "");
}

int VadereApi::VaderSimulationScope::getDepartedNumber() const {
  return (int)myParent.getInt(CMD_GET_SIM_VARIABLE,
                              VAR_DEPARTED_VEHICLES_NUMBER, "");
}

std::vector<std::string> VadereApi::VaderSimulationScope::getDepartedIDList()
    const {
  return myParent.getStringVector(CMD_GET_SIM_VARIABLE,
                                  VAR_DEPARTED_VEHICLES_IDS, "");
}

int VadereApi::VaderSimulationScope::getArrivedNumber() const {
  return (int)myParent.getInt(CMD_GET_SIM_VARIABLE, VAR_ARRIVED_VEHICLES_NUMBER,
                              "");
}

std::vector<std::string> VadereApi::VaderSimulationScope::getArrivedIDList()
    const {
  return myParent.getStringVector(CMD_GET_SIM_VARIABLE,
                                  VAR_ARRIVED_VEHICLES_IDS, "");
}

double VadereApi::VaderSimulationScope::getDeltaT() const {
  return myParent.getDouble(CMD_GET_SIM_VARIABLE, VAR_DELTA_T, "");
}

libsumo::TraCIPositionVector VadereApi::VaderSimulationScope::getNetBoundary()
    const {
  return myParent.getPolygon(CMD_GET_SIM_VARIABLE, VAR_NET_BOUNDING_BOX, "");
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
  send_commandGetVariable(CMD_GET_SIM_VARIABLE, DISTANCE_REQUEST, "", &content);
  tcpip::Storage inMsg;
  processGET(inMsg, CMD_GET_SIM_VARIABLE, TYPE_DOUBLE);
  return inMsg.readDouble();
}

std::string VadereApi::VaderSimulationScope::getScenarioHash(
    const std::string& scenario) const {
  tcpip::Storage content;
  content.writeByte(TYPE_STRING);
  content.writeString(scenario);
  send_commandGetVariableExtLenghtField(
      myCmdGetID, rover::constants::VAR_CACHE_HASH, "", &content);

  tcpip::Storage inMsg;
  processGET(inMsg, myCmdGetID, TYPE_STRING);
  return inMsg.readString();
}

CoordRef VadereApi::VaderSimulationScope::getCoordRef() const {
  send_commandGetVariable(myCmdGetID, constants::VAR_COORD_REF, "");
  tcpip::Storage inMsg;
  processGET(inMsg, myCmdGetID, TYPE_COMPOUND, true);
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

  send_commandSetValueExtLenghtField(
      myCmdSetID, rover::constants::VAR_SIM_CONFIG, "", content);
  tcpip::Storage inMsg;
  check_resultState(inMsg, myCmdSetID);
}

} /* namespace rover */
