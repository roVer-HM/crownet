/*
 * VadereApi.h
 *
 *  Created on: Aug 6, 2020
 *      Author: sts
 */

#pragma once

#include <traci/API.h>
#include <traci/VariableTraits.h>
#include <traci/sumo/utils/traci/TraCIAPI.h>

#include "crownet/artery/traci/VadereUtils.h"
#include "crownet/artery/traci/TraCiForwarder.h"
#include "crownet/common/converter/OsgCoordinateConverter.h"



namespace crownet {
namespace constants {

// command: set connection priority (execution order)
constexpr int VAR_TARGET_LIST = 0xfe;
constexpr int VAR_CACHE_HASH = 0x7d;
constexpr int VAR_SIM_CONFIG = 0x7e;
constexpr int VAR_EXTERNAL_INPUT = 0x20;
constexpr int VAR_PERSON_STIMULUS = 0xfd;
constexpr int CMD_FILE_SEND = 0x75;
constexpr int VAR_ARRIVED_PEDESTRIANS_IDS = 0x7a;
constexpr int VAR_DEPARTED_PEDESTRIANS_IDS = 0x74;
constexpr int VAR_COORD_REF = 0x90;
//todo: (CM) add constexpr for sensor  VAR_ID

}  // namespace constants
}  // namespace crownet

namespace traci {
#define VAR_TRAIT(var_, type_)                                     \
  template <>                                                      \
  struct VariableTrait<var_> {                                     \
    using value_type = type_;                                      \
    using result_type = TraCIResultTrait<value_type>::result_type; \
  };

VAR_TRAIT(crownet::constants::VAR_TARGET_LIST, std::vector<std::string>)
#undef VAR_TRAIT
}  // namespace traci

using namespace traci;

namespace crownet {

struct CoordRef {
  std::string epsg_code;
  traci::TraCIPosition offset;
};

class VadereApi : public API, public TraCiForwarder {
 public:
  VadereApi();
  virtual ~VadereApi() {}

  virtual TraCIGeoPosition convertGeo(const TraCIPosition&) const override;
  virtual TraCIPosition convert2D(const TraCIGeoPosition&) const override;

  void setConverter(std::shared_ptr<OsgCoordinateConverter> _c) {
    converter = _c;
  }

  // TraCiForwarder
  virtual void forward(tcpip::Storage& msgIn,  tcpip::Storage& msgResponse) override;

 private:
  std::shared_ptr<OsgCoordinateConverter> converter;



 public:
  class VaderePersonScope : public TraCIScopeWrapper {
   public:
    VaderePersonScope(API& parent)
        : TraCIScopeWrapper(
              parent,
              libsumo::CMD_GET_PERSON_VARIABLE,
              libsumo::CMD_SET_PERSON_VARIABLE,
              libsumo::CMD_SUBSCRIBE_PERSON_VARIABLE,
              libsumo::CMD_SUBSCRIBE_PERSON_CONTEXT) {}
    virtual ~VaderePersonScope() {}

//    std::vector<std::string> getIDList() const override;
//    int getIDCount() const override;
    double getSpeed(const std::string& personID) const;
    libsumo::TraCIPosition getPosition(const std::string& personID) const;
    std::string getTypeID(const std::string& personID) const;
    double getAngle(const std::string& personID) const;
    libsumo::TraCIColor getColor(const std::string& personID) const;
    std::vector<std::string> getTargetList(const std::string& personID) const;

    // add?

    void setSpeed(const std::string& personID, double speed) const;
    void setType(const std::string& personID, const std::string& typeID) const;
    void setLength(const std::string& personID, double length) const;
    void setWidth(const std::string& personID, double width) const;
    void setHeight(const std::string& personID, double height) const;
    void setColor(const std::string& personID,
                  const libsumo::TraCIColor& c) const;
    void setTargetList(const std::string& personID,
                       std::vector<std::string> targets) const;
    void setInformation(const std::string& personID, double start_at,
                        double obsolete_at, std::string information) const;

   private:
    /// @brief invalidated copy constructor
    VaderePersonScope(const VaderePersonScope& src);

    /// @brief invalidated assignment operator
    VaderePersonScope& operator=(const VaderePersonScope& src);
  };

  class VaderSimulationScope : public TraCIScopeWrapper {
   public:
    VaderSimulationScope(API& parent)
        : TraCIScopeWrapper(parent,
                libsumo::CMD_GET_SIM_VARIABLE,
                libsumo::CMD_SET_SIM_VARIABLE,
                libsumo::CMD_SUBSCRIBE_SIM_VARIABLE,
                libsumo::CMD_SUBSCRIBE_SIM_CONTEXT) {}
    virtual ~VaderSimulationScope() {}

    int getCurrentTime() const;
    double getTime() const;
    int getLoadedNumber() const;
    std::vector<std::string> getLoadedIDList() const;
    int getDepartedNumber() const;
    std::vector<std::string> getDepartedIDList() const;
    int getArrivedNumber() const;
    std::vector<std::string> getArrivedIDList() const;
    double getDeltaT() const;
    libsumo::TraCIPositionVector getNetBoundary() const;
    double getDistance2D(double x1, double y1, double x2, double y2,
                         bool isGeo = false, bool isDriving = false);
    std::string getScenarioHash(const std::string& scenario) const;
    CoordRef getCoordRef() const;
    void sendSimulationConfig(const vadere::SimCfg& cfg) const;
    void send_control(const std::string& personID, std::string model, std::string metadata) const;
    //todo: (CM) add method to translate a DCD map into a traci packet.


//   private:
//    /// @brief invalidated assignment operator
//    VaderSimulationScope& operator=(const VaderSimulationScope& src) = delete;
  };

 public:
  VaderSimulationScope v_simulation;
  VaderePersonScope v_person;
};

} /* namespace crownet */
