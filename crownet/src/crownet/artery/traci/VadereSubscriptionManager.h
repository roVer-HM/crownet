/*
 * VadereSubscriptionManager.h
 *
 *  Created on: Aug 7, 2020
 *      Author: sts
 */

#pragma once

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <omnetpp/csimplemodule.h>
#include <traci/Listener.h>
#include <traci/SubscriptionManager.h>
#include "crownet/artery/traci/VadereApi.h"
#include "crownet/artery/traci/VadereLiteApi.h"
#include "crownet/artery/traci/VariableCache.h"

namespace crownet {

class VadereNodeManager;

class VadereSubscriptionManager : public traci::Listener,
                                  public traci::ISubscriptionManager,
                                  public omnetpp::cSimpleModule {
 public:
  friend VadereNodeManager;
  VadereSubscriptionManager();
  virtual ~VadereSubscriptionManager();

  void step() override;
  void subscribePersonVariables(const std::set<int>& personVariables);
  void subscribeSimulationVariables(const std::set<int>& simulationVariables);
  const std::unordered_set<std::string>& getSubscribedPersons() const;
  const std::unordered_map<std::string, std::shared_ptr<VaderePersonCache>>&
  getAllPersonCaches() const;
  std::shared_ptr<VaderePersonCache> getPersonCache(const std::string& id);
  std::shared_ptr<VadereSimulationCache> getSimulationCache();

 protected:
  void initialize() override;
  using omnetpp::cIListener::finish;  // [-Woverloaded-virtual]
  void finish() override;

  void subscribePerson(const std::string& id);
  void unsubscribePerson(const std::string& id, bool person_exists);
  void updatePersonSubscription(const std::string& id,
                                const std::vector<int>& vars);

 private:
  VadereSubscriptionManager(const VadereSubscriptionManager&) = delete;

  // implement traci::Listener
  void traciInit() override;
  void traciStep() override;
  void traciClose() override;

  VadereLiteApi* m_api;
  std::unordered_set<std::string> m_subscribed_persons;
  std::vector<int> m_person_vars;
  std::vector<int> m_sim_vars;
  std::unordered_map<std::string, std::shared_ptr<VaderePersonCache>>
      m_person_caches;
  std::shared_ptr<VadereSimulationCache> m_sim_cache;
};

} /* namespace crownet */
