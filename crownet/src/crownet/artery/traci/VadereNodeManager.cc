/*
 * VadereNodeManager.cc
 *
 *  Created on: Aug 7, 2020
 *      Author: sts
 */

#include "VadereNodeManager.h"
#include <inet/common/ModuleAccess.h>
#include "inet/common/scenario/ScenarioManager.h"
#include "crownet/artery/traci/VadereCore.h"

using namespace crownet::constants;
using namespace libsumo;

namespace crownet {

namespace {
static const std::set<int> sVehicleVariables{VAR_POSITION, VAR_SPEED,
                                             VAR_ANGLE};
static const std::set<int> sSimulationVariables{
    VAR_TIME, VAR_ARRIVED_PEDESTRIANS_IDS, VAR_DEPARTED_PEDESTRIANS_IDS};

class VaderePersonObjectImpl : public VadereNodeManager::VaderePersonObject {
 public:
  VaderePersonObjectImpl(std::shared_ptr<VaderePersonCache> cache)
      : m_cache(cache) {}

  std::shared_ptr<VaderePersonCache> getCache() const override {
    return m_cache;
  }
  const TraCIPosition& getPosition() const override {
    return m_cache->get<VAR_POSITION>();
  }
  TraCIAngle getHeading() const override {
    return TraCIAngle{m_cache->get<VAR_ANGLE>()};
  }
  double getSpeed() const override { return m_cache->get<VAR_SPEED>(); }

 private:
  std::shared_ptr<VaderePersonCache> m_cache;
};
}  // namespace

Define_Module(VadereNodeManager);

const simsignal_t VadereNodeManager::addNodeSignal =
    cComponent::registerSignal("traci.node.add");
const simsignal_t VadereNodeManager::updateNodeSignal =
    cComponent::registerSignal("traci.node.update");
const simsignal_t VadereNodeManager::removeNodeSignal =
    cComponent::registerSignal("traci.node.remove");
const simsignal_t VadereNodeManager::addPersonSignal =
    cComponent::registerSignal("traci.person.add");
const simsignal_t VadereNodeManager::updatePersonSignal =
    cComponent::registerSignal("traci.person.update");
const simsignal_t VadereNodeManager::removePersonSignal =
    cComponent::registerSignal("traci.person.remove");

void VadereNodeManager::initialize() {
  VadereCore* core =
      inet::getModuleFromPar<VadereCore>(par("coreModule"), this);
  subscribeTraCI(core);
  m_api = std::dynamic_pointer_cast<VadereApi>(core->getAPI());
  m_mapper = inet::getModuleFromPar<ModuleMapper>(par("mapperModule"), this);
  m_nodeIndex = 0;
  m_objectSinkModule = par("objectSinkModule").stringValue();
  m_subscriptions = inet::getModuleFromPar<VadereSubscriptionManager>(
      par("subscriptionsModule"), this);
}

void VadereNodeManager::finish() {
  m_api = nullptr;
  unsubscribeTraCI();
  cSimpleModule::finish();
}

void VadereNodeManager::traciInit() {

  m_boundary = Boundary{m_api->v_simulation.getNetBoundary()};
  m_subscriptions->subscribeSimulationVariables(sSimulationVariables);
  m_subscriptions->subscribePersonVariables(sVehicleVariables);

  // insert and subscribe to already running nodes
  for (const std::string& id : m_api->v_person.getIDList()) {
    addMovingObject(id);
    m_subscriptions->subscribePerson(id);
  }
}

void VadereNodeManager::traciStep() {
  auto sim_cache = m_subscriptions->getSimulationCache();
  // todo
  //    ASSERT(checkTimeSync(*sim_cache, omnetpp::simTime()));

  const auto& departed = sim_cache->get<VAR_DEPARTED_VEHICLES_IDS>();
  EV_DETAIL << "TraCI: " << departed.size() << " persons departed" << endl;
  for (const auto& id : departed) {
    addMovingObject(id);
  }

  const auto& arrived = sim_cache->get<VAR_ARRIVED_VEHICLES_IDS>();
  EV_DETAIL << "TraCI: " << arrived.size() << " persons arrived" << endl;
  for (const auto& id : arrived) {
    removeMovingObject(id);
  }

  for (auto& obj : m_persons) {
    const std::string& id = obj.first;
    VaderePersonSink* sink = obj.second;
    updateMovingObject(id, sink);
  }

  emit(updateNodeSignal, getNumberOfNodes());
}

void VadereNodeManager::traciClose() {
  for (unsigned i = m_nodes.size(); i > 0; --i) {
    removeNodeModule(m_nodes.begin()->first);
  }
}

void VadereNodeManager::addMovingObject(const std::string& id) {
  NodeInitializer init = [this, &id](cModule* module) {
      VaderePersonSink* objectSink = getObjectSink(module);
    auto& traci = m_api->v_person;
    objectSink->initializeSink(m_api, m_subscriptions->getPersonCache(id), m_boundary);
    objectSink->initializePerson(traci.getPosition(id),
                                 TraCIAngle{traci.getAngle(id)},
                                 traci.getSpeed(id));
    m_persons[id] = objectSink;
  };

  emit(addPersonSignal, id.c_str());
  //fixme implement mapper for vadere person
  cModuleType* type = m_mapper->person(*this, id);
  if (type != nullptr) {
    addNodeModule(id, type, init);
  } else {
    m_persons[id] = nullptr;
  }
}

void VadereNodeManager::removeMovingObject(const std::string& id) {
  emit(removePersonSignal, id.c_str());
  removeNodeModule(id);
  m_persons.erase(id);
}

void VadereNodeManager::updateMovingObject(const std::string& id,
        VaderePersonSink* sink) {
  auto person = m_subscriptions->getPersonCache(id);
  VaderePersonObjectImpl update(person);
  emit(updatePersonSignal, id.c_str(), &update);
  if (sink) {
    sink->updatePerson(person->get<VAR_POSITION>(),
                       TraCIAngle{person->get<VAR_ANGLE>()},
                       person->get<VAR_SPEED>());
  }
}

cModule* VadereNodeManager::createModule(const std::string&,
                                         cModuleType* type) {
  cModule* module =
      type->create("node", getSystemModule(), m_nodeIndex, m_nodeIndex);
  ++m_nodeIndex;
  return module;
}

cModule* VadereNodeManager::addNodeModule(const std::string& id,
                                          cModuleType* type,
                                          NodeInitializer& init) {
  cModule* module = createModule(id, type);
  module->finalizeParameters();
  module->buildInside();
  m_nodes[id] = module;
  init(module);
  module->scheduleStart(simTime());

  inet::cPreModuleInitNotification pre;
  pre.module = module;
  emit(POST_MODEL_CHANGE, &pre);
  module->callInitialize();
  inet::cPostModuleInitNotification post;
  post.module = module;
  emit(POST_MODEL_CHANGE, &post);

  emit(addNodeSignal, id.c_str(), module);

  return module;
}

void VadereNodeManager::removeNodeModule(const std::string& id) {
  cModule* module = getNodeModule(id);
  if (module) {
    emit(removeNodeSignal, id.c_str(), module);
    module->callFinish();
    module->deleteModule();
    m_nodes.erase(id);
  } else {
    EV_DEBUG << "Node with id " << id << " does not exist, no removal\n";
  }
}

cModule* VadereNodeManager::getNodeModule(const std::string& id) {
  auto found = m_nodes.find(id);
  return found != m_nodes.end() ? found->second : nullptr;
}

std::size_t VadereNodeManager::getNumberOfNodes() const {
  return m_nodes.size();
}

VaderePersonSink* VadereNodeManager::getObjectSink(cModule* node) {
  ASSERT(node);
  cModule* module = node->getModuleByPath(m_objectSinkModule.c_str());
  if (!module) {
    throw cRuntimeError("No module found at %s relative to %s",
                        m_objectSinkModule.c_str(),
                        node->getFullPath().c_str());
  }

  auto* mobility = dynamic_cast<VaderePersonSink*>(module);
  if (!mobility) {
    throw cRuntimeError("Module %s is not a VehicleSink",
                        module->getFullPath().c_str());
  }

  return mobility;
}

VaderePersonSink* VadereNodeManager::getObjectSink(const std::string& id) {
  auto found = m_persons.find(id);
  return found != m_persons.end() ? found->second : nullptr;
}

}  // namespace crownet
