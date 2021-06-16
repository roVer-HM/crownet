/*
 * VadereNodeManager.h
 *
 *  Created on: Aug 7, 2020
 *      Author: sts
 */

#pragma once

#include <omnetpp/ccomponent.h>
#include <traci/Listener.h>
#include <traci/ModuleMapper.h>
#include <traci/NodeManager.h>
#include <traci/VariableCache.h>
#include <functional>
#include "crownet/artery/traci.h"
#include "crownet/artery/traci/VadereSubscriptionManager.h"
#include "crownet/artery/traci/VariableCache.h"
#include "crownet/artery/traci/VaderePersonSink.h"

namespace crownet {

class VadereNodeManager : public traci::NodeManager,
                          public traci::Listener,
                          public omnetpp::cSimpleModule {
 public:
  static const omnetpp::simsignal_t addNodeSignal;
  static const omnetpp::simsignal_t updateNodeSignal;
  static const omnetpp::simsignal_t removeNodeSignal;
  static const omnetpp::simsignal_t addPersonSignal;
  static const omnetpp::simsignal_t updatePersonSignal;
  static const omnetpp::simsignal_t removePersonSignal;

  std::shared_ptr<API> getAPI() override { return m_api; }
  std::size_t getNumberOfNodes() const override;

  class VaderePersonObject : public traci::NodeManager::MovingObject {
   public:
    virtual std::shared_ptr<VaderePersonCache> getCache() const = 0;
  };

 public:
  virtual ~VadereNodeManager() = default;

 protected:
  using NodeInitializer = std::function<void(omnetpp::cModule*)>;

  void initialize() override;
//  using omnetpp::cIListener::finish;  // [-Woverloaded-virtual]
  void finish() override;

  virtual void addMovingObject(const std::string&);
  virtual void removeMovingObject(const std::string&);
  virtual void updateMovingObject(const std::string&, VaderePersonSink*);
  virtual omnetpp::cModule* createModule(const std::string&,
                                         omnetpp::cModuleType*);
  virtual omnetpp::cModule* addNodeModule(const std::string&,
                                          omnetpp::cModuleType*,
                                          NodeInitializer&);
  virtual void removeNodeModule(const std::string&);
  virtual omnetpp::cModule* getNodeModule(const std::string&);
  virtual VaderePersonSink* getObjectSink(omnetpp::cModule*);
  virtual VaderePersonSink* getObjectSink(const std::string&);

 private:
  void traciInit() override;
  void traciStep() override;
  void traciClose() override;

  std::shared_ptr<VadereApi> m_api;
  ModuleMapper* m_mapper;
  Boundary m_boundary;
  VadereSubscriptionManager* m_subscriptions;
  unsigned m_nodeIndex;
  std::map<std::string, VaderePersonSink*> m_persons;
  std::string m_personSinkModule;
};

} /* namespace crownet */
