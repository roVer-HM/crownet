/*
 * VadereSubscriptionManager.cc
 *
 *  Created on: Aug 7, 2020
 *      Author: sts
 */

#include "VadereSubscriptionManager.h"

#include <omnetpp.h>
#include "inet/common/ModuleAccess.h"
#include "rover/artery/traci/VadereCore.h"

using namespace omnetpp;

namespace rover {

Define_Module(VadereSubscriptionManager);

VadereSubscriptionManager::VadereSubscriptionManager() : m_api(nullptr) {}

VadereSubscriptionManager::~VadereSubscriptionManager() {}

void VadereSubscriptionManager::step() {
  const auto& simvars = m_api->vSimulation().getSubscriptionResults("");
  m_sim_cache->reset(simvars);
  // todo check timeSync
  // ASSERT(traci::checkTimeSync(*m_sim_cache, omnetpp::simTime()));

  const auto& arrivedVehicles = m_sim_cache->get<VAR_ARRIVED_VEHICLES_IDS>();
  for (const auto& id : arrivedVehicles) {
    unsubscribePerson(id, false);
  }

  const auto& departedVehicles = m_sim_cache->get<VAR_DEPARTED_VEHICLES_IDS>();
  for (const auto& id : departedVehicles) {
    subscribePerson(id);
  }

  const auto& persons = m_api->vPerson();
  for (const std::string& person : m_subscribed_persons) {
    const auto& vars = persons.getSubscriptionResults(person);
    getPersonCache(person)->reset(vars);
  }
}

void VadereSubscriptionManager::subscribePersonVariables(
    const std::set<int>& add_vars) {
  std::vector<int> tmp_vars;
  std::set_union(m_person_vars.begin(), m_person_vars.end(), add_vars.begin(),
                 add_vars.end(), std::back_inserter(tmp_vars));
  std::swap(m_person_vars, tmp_vars);
  ASSERT(m_person_vars.size() >= tmp_vars.size());

  if (m_person_vars.size() != tmp_vars.size()) {
    for (const std::string& person : m_subscribed_persons) {
      updatePersonSubscription(person, m_person_vars);
    }
  }
}

void VadereSubscriptionManager::subscribeSimulationVariables(
    const std::set<int>& add_vars) {
  std::vector<int> tmp_vars;
  std::set_union(m_sim_vars.begin(), m_sim_vars.end(), add_vars.begin(),
                 add_vars.end(), std::back_inserter(tmp_vars));
  std::swap(m_sim_vars, tmp_vars);
  ASSERT(m_sim_vars.size() >= tmp_vars.size());

  if (m_sim_vars.size() != tmp_vars.size()) {
    m_api->simulation().subscribe("", m_sim_vars, INVALID_DOUBLE_VALUE,
                                  INVALID_DOUBLE_VALUE);
  }
}

const std::unordered_set<std::string>&
VadereSubscriptionManager::getSubscribedPersons() const {
  return m_subscribed_persons;
}

const std::unordered_map<std::string, std::shared_ptr<VaderePersonCache>>&
VadereSubscriptionManager::getAllPersonCaches() const {
  return m_person_caches;
}

std::shared_ptr<VaderePersonCache> VadereSubscriptionManager::getPersonCache(
    const std::string& id) {
  auto found = m_person_caches.find(id);
  if (found == m_person_caches.end()) {
    std::tie(found, std::ignore) = m_person_caches.emplace(
        id, std::make_shared<VaderePersonCache>(*m_api, id));
  }
  return found->second;
}

std::shared_ptr<VadereSimulationCache>
VadereSubscriptionManager::getSimulationCache() {
  ASSERT(m_sim_cache);
  return m_sim_cache;
}

void VadereSubscriptionManager::initialize() {
  VadereCore* core =
      inet::getModuleFromPar<VadereCore>(par("coreModule"), this);
  subscribeTraCI(core);
  auto _api = &core->getLiteAPI();
  m_api = check_and_cast<VadereLiteApi*>(_api);
  m_sim_cache = std::make_shared<VadereSimulationCache>(*m_api);
}

void VadereSubscriptionManager::finish() {
  m_api = nullptr;
  unsubscribeTraCI();
  cSimpleModule::finish();
}

void VadereSubscriptionManager::traciInit() {
  using namespace traci::constants;
  static const std::set<int> vars{VAR_DELTA_T, VAR_TIME};
  subscribeSimulationVariables(vars);

  // subscribe already running vehicles
  if (!m_person_vars.empty()) {
    for (const std::string& id : m_api->vPerson().getIDList()) {
      subscribePerson(id);
    }
  }
}

void VadereSubscriptionManager::traciStep() {}

void VadereSubscriptionManager::traciClose() {}

void VadereSubscriptionManager::subscribePerson(const std::string& id) {
  updatePersonSubscription(id, m_person_vars);
  m_subscribed_persons.insert(id);
}

void VadereSubscriptionManager::unsubscribePerson(const std::string& id,
                                                  bool person_exists) {
  if (person_exists) {
    static const std::vector<int> empty;
    updatePersonSubscription(id, empty);
  }
  m_subscribed_persons.erase(id);
}

void VadereSubscriptionManager::updatePersonSubscription(
    const std::string& id, const std::vector<int>& vars) {
  m_api->vPerson().subscribe(id, vars, INVALID_DOUBLE_VALUE,
                             INVALID_DOUBLE_VALUE);
}

} /* namespace rover */
