/*
 * VaderePersonController.cc
 *
 *  Created on: Aug 10, 2020
 *      Author: sts
 */

#include "VaderePersonController.h"
#include <omnetpp/checkandcast.h>
#include <artery/traci/Cast.h>
#include <boost/units/systems/angle/degrees.hpp>
#include <boost/units/systems/si/acceleration.hpp>
#include <boost/units/systems/si/plane_angle.hpp>
#include <boost/units/systems/si/velocity.hpp>

namespace si = boost::units::si;

using namespace crownet::constants;
using namespace libsumo;

namespace crownet {

VaderePersonController::VaderePersonController(const std::string& id,
        std::shared_ptr<API> api)
    : VaderePersonController(std::make_shared<VaderePersonCache>(api, id)) {}

VaderePersonController::VaderePersonController(
    std::shared_ptr<VaderePersonCache> cache)
    : VaderePersonController(cache->getId(), cache, cache->getApi()) {}

VaderePersonController::VaderePersonController(
    const std::string& id, std::shared_ptr<VaderePersonCache> cache,
    std::shared_ptr<API> api)
    : m_id(id),
      m_cache(cache),
      m_api(std::dynamic_pointer_cast<VadereApi>(api)),
      m_boundary(m_api->v_simulation.getNetBoundary()) {}

const std::string& VaderePersonController::getNodeId() const { return m_id; }
std::string VaderePersonController::getTypeId() const {
  return m_cache->get<VAR_TYPE>();
}

const std::string VaderePersonController::getNodeClass() const {
  return m_cache->get<VAR_TYPE>();  // todo  VAR_VEHICLECLASS -> VAR_PersonCLASS
}

artery::Position VaderePersonController::getPosition() const {
  return artery::position_cast(m_boundary, m_cache->get<VAR_POSITION>());
}

auto VaderePersonController::getGeoPosition() const -> artery::GeoPosition {
  TraCIPosition traci_pos = m_cache->get<VAR_POSITION>();

  TraCIGeoPosition traci_geo = m_api->convertGeo(traci_pos);
  artery::GeoPosition geo;
  geo.latitude = traci_geo.latitude * boost::units::degree::degree;
  geo.longitude = traci_geo.longitude * boost::units::degree::degree;
  return geo;
}

auto VaderePersonController::getHeading() const -> artery::Angle {
  return artery::angle_cast(TraCIAngle{m_cache->get<VAR_ANGLE>()});
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
  m_api->v_person.setSpeed(m_id, v / si::meter_per_second);
}

auto VaderePersonController::getLength() const -> Length {
  return m_cache->get<VAR_LENGTH>() * si::meter;
}

auto VaderePersonController::getWidth() const -> Length {
  return m_cache->get<VAR_WIDTH>() * si::meter;
}

std::vector<std::string> VaderePersonController::getTargetList() const {
  return m_cache->get<VAR_TARGET_LIST>();
}

void VaderePersonController::setTargetList(std::vector<std::string> targetId) {
  m_api->v_person.setTargetList(m_id, targetId);

  m_cache->invalidate(VAR_TARGET_LIST);
}

void VaderePersonController::appendTarget(const std::string& targetId,
                                          bool back) {
  auto target = m_api->v_person.getTargetList(m_id);
  if (back) {
    target.push_back(targetId);
  } else {
    target.insert(target.begin(), targetId);
  }
  m_cache->invalidate(VAR_TARGET_LIST);
}

void VaderePersonController::setInformed(const simtime_t& start,
                                         const simtime_t& obsolte_at,
                                         const std::string& data) {
  m_api->v_person.setInformation(m_id, start.dbl(), obsolte_at.dbl(), data);
}

void VaderePersonController::send_control(std::string model, std::string metadata){
    m_api->v_simulation.send_control(m_id, model, metadata);
}

} /* namespace crownet */
