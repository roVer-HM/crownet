/*
 * VaderePersonController.cc
 *
 *  Created on: Aug 10, 2020
 *      Author: sts
 */

#include "VaderePersonController.h"
#include <omnetpp/checkandcast.h>
#include <boost/units/systems/angle/degrees.hpp>
#include <boost/units/systems/si/acceleration.hpp>
#include <boost/units/systems/si/plane_angle.hpp>
#include <boost/units/systems/si/velocity.hpp>

namespace si = boost::units::si;

namespace rover {

VaderePersonController::VaderePersonController(const std::string& id,
                                               rover::VadereLiteApi& api)
    : VaderePersonController(std::make_shared<VaderePersonCache>(api, id)) {}

VaderePersonController::VaderePersonController(
    std::shared_ptr<VaderePersonCache> cache)
    : VaderePersonController(cache->getId(), cache, cache->getLiteAPI()) {}

VaderePersonController::VaderePersonController(
    const std::string& id, std::shared_ptr<VaderePersonCache> cache,
    traci::LiteAPI& api)
    : m_id(id),
      m_cache(cache),
      m_api(dynamic_cast<VadereLiteApi&>(api)),
      m_boundary(m_api.vSimulation().getNetBoundary()) {}

const std::string& VaderePersonController::getNodeId() const { return m_id; }
std::string VaderePersonController::getTypeId() const {
  return m_cache->get<VAR_TYPE>();
}

const std::string VaderePersonController::getNodeClass() const {
  return m_cache->get<VAR_TYPE>();  // todo  VAR_VEHICLECLASS -> VAR_PersonCLASS
}

artery::Position VaderePersonController::getPosition() const {
  return traci::position_cast(m_boundary, m_cache->get<VAR_POSITION>());
}

auto VaderePersonController::getGeoPosition() const -> artery::GeoPosition {
  TraCIPosition traci_pos = m_cache->get<VAR_POSITION>();

  TraCIGeoPosition traci_geo = m_api.convertGeo(traci_pos);
  artery::GeoPosition geo;
  geo.latitude = traci_geo.latitude * boost::units::degree::degree;
  geo.longitude = traci_geo.longitude * boost::units::degree::degree;
  return geo;
}

auto VaderePersonController::getHeading() const -> artery::Angle {
  using namespace traci;
  return angle_cast(TraCIAngle{m_cache->get<VAR_ANGLE>()});
}
auto VaderePersonController::getSpeed() const -> Velocity {
  return m_cache->get<VAR_SPEED>() * si::meter_per_second;
}

auto VaderePersonController::getMaxSpeed() const -> Velocity {
  throw std::logic_error("Not implemented");
}

void VaderePersonController::setMaxSpeed(Velocity v) {
  throw std::logic_error("Not implemented");
}

void VaderePersonController::setSpeed(Velocity v) {
  m_api.vPerson().setSpeed(m_id, v / si::meter_per_second);
}

auto VaderePersonController::getLength() const -> Length {
  return m_cache->get<VAR_LENGTH>() * si::meter;
}

auto VaderePersonController::getWidth() const -> Length {
  return m_cache->get<VAR_WIDTH>() * si::meter;
}

} /* namespace rover */
