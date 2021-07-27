//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "SumoCombinedNodeManager.h"
#include <omnetpp/ccomponent.h>
#include <omnetpp/csimplemodule.h>
#include <inet/common/ModuleAccess.h>
#include "traci/API.h"
#include "traci/BasicNodeManager.h"
#include "traci/CheckTimeSync.h"
#include "traci/Core.h"
#include "traci/ModuleMapper.h"
#include "traci/PersonSink.h"
#include "traci/VariableCache.h"
#include "traci/VehicleSink.h"
#include "inet/common/scenario/ScenarioManager.h"

using namespace omnetpp;
using namespace traci;

namespace
{
static const std::set<int> sPersonVariables {
    libsumo::VAR_POSITION, libsumo::VAR_SPEED, libsumo::VAR_ANGLE
};
static const std::set<int> sVehicleVariables {
    libsumo::VAR_POSITION, libsumo::VAR_SPEED, libsumo::VAR_ANGLE
};
static const std::set<int> sSimulationVariables {
    libsumo::VAR_DEPARTED_VEHICLES_IDS, libsumo::VAR_ARRIVED_VEHICLES_IDS, libsumo::VAR_TELEPORT_STARTING_VEHICLES_IDS,
    libsumo::VAR_TIME
};
class VehicleObjectImpl : public BasicNodeManager::VehicleObject
{
public:
    VehicleObjectImpl(std::shared_ptr<VehicleCache> cache) : m_cache(cache) {}

    std::shared_ptr<VehicleCache> getCache() const override { return m_cache; }
    const TraCIPosition& getPosition() const override { return m_cache->get<libsumo::VAR_POSITION>(); }
    TraCIAngle getHeading() const override { return TraCIAngle { m_cache->get<libsumo::VAR_ANGLE>() }; }
    double getSpeed() const override { return m_cache->get<libsumo::VAR_SPEED>(); }

private:
    std::shared_ptr<VehicleCache> m_cache;
};

class PersonObjectImpl : public BasicNodeManager::PersonObject
{
public:
    PersonObjectImpl(std::shared_ptr<PersonCache> cache) : m_cache(cache) {}

    std::shared_ptr<PersonCache> getCache() const override { return m_cache; }
    const TraCIPosition& getPosition() const override { return m_cache->get<libsumo::VAR_POSITION>(); }
    TraCIAngle getHeading() const override { return TraCIAngle { m_cache->get<libsumo::VAR_ANGLE>() }; }
    double getSpeed() const override { return m_cache->get<libsumo::VAR_SPEED>(); }

private:
    std::shared_ptr<PersonCache> m_cache;
};

} // namespace

namespace  crownet {

Define_Module(SumoCombinedNodeManager);

const simsignal_t SumoCombinedNodeManager::addNodeSignal = cComponent::registerSignal("traci.node.add");
const simsignal_t SumoCombinedNodeManager::updateNodeSignal = cComponent::registerSignal("traci.node.update");
const simsignal_t SumoCombinedNodeManager::removeNodeSignal = cComponent::registerSignal("traci.node.remove");
const simsignal_t SumoCombinedNodeManager::addPersonSignal = cComponent::registerSignal("traci.person.add");
const simsignal_t SumoCombinedNodeManager::updatePersonSignal = cComponent::registerSignal("traci.person.update");
const simsignal_t SumoCombinedNodeManager::removePersonSignal = cComponent::registerSignal("traci.person.remove");
const simsignal_t SumoCombinedNodeManager::addVehicleSignal = cComponent::registerSignal("traci.vehicle.add");
const simsignal_t SumoCombinedNodeManager::updateVehicleSignal = cComponent::registerSignal("traci.vehicle.update");
const simsignal_t SumoCombinedNodeManager::removeVehicleSignal = cComponent::registerSignal("traci.vehicle.remove");



SumoCombinedNodeManager::SumoCombinedNodeManager() { }

SumoCombinedNodeManager::~SumoCombinedNodeManager() {}


void SumoCombinedNodeManager::initialize()
{
    Core* core = inet::getModuleFromPar<Core>(par("coreModule"), this);
    subscribeTraCI(core);
    m_api = core->getAPI();
    m_mapper = inet::getModuleFromPar<ModuleMapper>(par("mapperModule"), this);
    m_subscriptions = inet::getModuleFromPar<SubscriptionManager>(par("subscriptionsModule"), this);

    // person setup
    m_person_sink_module = par("personSinkModule").stringValue();
    m_ignore_persons = par("ignorePersons").boolValue();
    m_person_module_vector = par("personNode").stringValue();

    // vehicle setup
    m_vehicle_sink_module = par("vehicleSinkModule").stringValue();
    m_ignore_vehicle = par("ignoreVehicle").boolValue();
    m_vehicle_module_vector = par("vehicleNode").stringValue();
    m_destroy_vehicles_on_crash = par("destroyVehiclesOnCrash");

    m_nodeIndex[m_vehicle_module_vector] = 0;
    m_nodeIndex[m_person_module_vector] = 0;
}

void SumoCombinedNodeManager::finish()
{
    unsubscribeTraCI();
    cSimpleModule::finish();
}

void SumoCombinedNodeManager::traciInit()
{
    m_boundary = Boundary { m_api->simulation.getNetBoundary() };
    m_subscriptions->subscribeSimulationVariables(sSimulationVariables);

    // insert already running vehicles
    if(!m_ignore_vehicle){
        for (const std::string& id : m_api->vehicle.getIDList()) {
            addVehicle(id);
        }
        m_subscriptions->subscribeVehicleVariables(sVehicleVariables);
    }

    // initialize persons if enabled
    if (!m_ignore_persons) {
        m_subscriptions->subscribePersonVariables(sPersonVariables);

        // insert already running persons
        for (const std::string& id : m_api->person.getIDList()) {
            addPerson(id);
        }
    }

    // treat SUMO start time as constant offset
    m_offset = omnetpp::SimTime { m_api->simulation.getCurrentTime(), omnetpp::SIMTIME_MS };
    m_offset -= omnetpp::simTime();
}

void SumoCombinedNodeManager::traciStep()
{
    if (!m_ignore_vehicle){
        processVehicles();
    }
    if (!m_ignore_persons) {
        processPersons();
    }
    emit(updateNodeSignal, getNumberOfNodes());
}


void SumoCombinedNodeManager::traciClose()
{
    for (unsigned i = m_personNodes.size(); i > 0; --i) {
        removePersonNodeModule(m_persons.begin()->first);
    }
    for (unsigned i = m_vehicleNodes.size(); i > 0; --i) {
        removeVehicleNodeModule(m_vehicleNodes.begin()->first);
    }
}

void SumoCombinedNodeManager::processVehicles()
{
    auto sim_cache = m_subscriptions->getSimulationCache();
    ASSERT(checkTimeSync(*sim_cache, omnetpp::simTime() + m_offset));

    const auto& departed = sim_cache->get<libsumo::VAR_DEPARTED_VEHICLES_IDS>();
    EV_DETAIL << "TraCI: " << departed.size() << " vehicles departed" << endl;
    for (const auto& id : departed) {
        addVehicle(id);
    }

    const auto& arrived = sim_cache->get<libsumo::VAR_ARRIVED_VEHICLES_IDS>();
    EV_DETAIL << "TraCI: " << arrived.size() << " vehicles arrived" << endl;
    for (const auto& id : arrived) {
        removeVehicle(id);
    }

    if (m_destroy_vehicles_on_crash) {
        const auto& teleport = sim_cache->get<libsumo::VAR_TELEPORT_STARTING_VEHICLES_IDS>();
        for (const auto& id : teleport) {
            EV_DETAIL << "TraCI: " << id << " got teleported and is removed!" << endl;
            removeVehicle(id);
        }
    }

    for (auto& vehicle : m_vehicles) {
        const std::string& id = vehicle.first;
        VehicleSink* sink = vehicle.second;
        updateVehicle(id, sink);
    }
}

void SumoCombinedNodeManager::addVehicle(const std::string& id)
{
    NodeInitializer init = [this, &id](cModule* module) {
        VehicleSink* vehicle = getVehicleSink(module);
        auto& traci = m_api->vehicle;
        vehicle->initializeSink(m_api, m_subscriptions->getVehicleCache(id), m_boundary);
        vehicle->initializeVehicle(traci.getPosition(id), TraCIAngle { traci.getAngle(id) }, traci.getSpeed(id));
        m_vehicles[id] = vehicle;
    };

    emit(addVehicleSignal, id.c_str());
    cModuleType* type = m_mapper->vehicle(*this, id);
    if (type != nullptr) {
        addNodeModule(id, type, init, m_vehicle_module_vector);
    } else {
        m_vehicles[id] = nullptr;
    }
}

void SumoCombinedNodeManager::removeVehicle(const std::string& id)
{
    emit(removeVehicleSignal, id.c_str());
    removeVehicleNodeModule(id);
    m_vehicles.erase(id);
}

void SumoCombinedNodeManager::updateVehicle(const std::string& id, VehicleSink* sink)
{
    auto vehicle = m_subscriptions->getVehicleCache(id);
    VehicleObjectImpl update(vehicle);
    emit(updateVehicleSignal, id.c_str(), &update);
    if (sink) {
        sink->updateVehicle(vehicle->get<libsumo::VAR_POSITION>(),
                TraCIAngle { vehicle->get<libsumo::VAR_ANGLE>() },
                vehicle->get<libsumo::VAR_SPEED>());
    }
}

void SumoCombinedNodeManager::processPersons()
{
    auto sim_cache = m_subscriptions->getSimulationCache();

    const auto& departed = sim_cache->get<libsumo::VAR_DEPARTED_PERSONS_IDS>();
    EV_DETAIL << "TraCI: " << departed.size() << " persons departed" << endl;
    for (const auto& id : departed) {
        addPerson(id);
    }

    const auto& arrived = sim_cache->get<libsumo::VAR_ARRIVED_PERSONS_IDS>();
    EV_DETAIL << "TraCI: " << arrived.size() << " persons arrived" << endl;
    for (const auto& id : arrived) {
        removePerson(id);
    }

    for (auto& person : m_persons) {
        const std::string& id = person.first;
        PersonSink* sink = person.second;
        updatePerson(id, sink);
    }
}

void SumoCombinedNodeManager::addPerson(const std::string& id)
{
    NodeInitializer init = [this, &id](cModule* module) {
        PersonSink* person = getPersonSink(module);
        auto& traci = m_api->person;
        person->initializeSink(m_api, m_subscriptions->getPersonCache(id), m_boundary);
        person->initializePerson(traci.getPosition(id), TraCIAngle { traci.getAngle(id) }, traci.getSpeed(id));
        m_persons[id] = person;
    };

    emit(addPersonSignal, id.c_str());
    cModuleType* type = m_mapper->person(*this, id);
    if (type != nullptr) {
        addNodeModule(id, type, init, m_person_module_vector);
    } else {
        m_persons[id] = nullptr;
    }
}

void SumoCombinedNodeManager::removePerson(const std::string& id)
{
    emit(removePersonSignal, id.c_str());
    removePersonNodeModule(id);
    m_persons.erase(id);
}

cModule* SumoCombinedNodeManager::createModule(const std::string&, cModuleType* type, const std::string& moduleVector)
{
    cModule* module = type->create(moduleVector.c_str(), getSystemModule(), m_nodeIndex[moduleVector], m_nodeIndex[moduleVector]);
    ++m_nodeIndex[moduleVector];
    return module;
}

cModule* SumoCombinedNodeManager::addNodeModule(const std::string& id, cModuleType* type, NodeInitializer& init, const std::string& moduleVector)
{
    cModule* module = createModule(id, type, moduleVector);
    module->finalizeParameters();
    module->buildInside();
    getNodeVector(moduleVector)[id] = module;
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

void SumoCombinedNodeManager::updatePerson(const std::string& id, PersonSink* sink)
{
    auto person = m_subscriptions->getPersonCache(id);
    PersonObjectImpl update(person);
    emit(updatePersonSignal, id.c_str(), &update);
    if (sink) {
        sink->updatePerson(person->get<libsumo::VAR_POSITION>(),
                TraCIAngle { person->get<libsumo::VAR_ANGLE>() },
                person->get<libsumo::VAR_SPEED>());
    }
}


void SumoCombinedNodeManager::removePersonNodeModule(const std::string& id)
{
    cModule* module = getPersonNodeModule(id);
    if (module) {
        emit(removeNodeSignal, id.c_str(), module);
        module->callFinish();
        module->deleteModule();
        m_personNodes.erase(id);
    } else {
        EV_DEBUG << "Node with id " << id << " does not exist, no removal\n";
    }
}

void SumoCombinedNodeManager::removeVehicleNodeModule(const std::string& id)
{
    cModule* module = getVehicleNodeModule(id);
    if (module) {
        emit(removeNodeSignal, id.c_str(), module);
        module->callFinish();
        module->deleteModule();
        m_vehicleNodes.erase(id);
    } else {
        EV_DEBUG << "Node with id " << id << " does not exist, no removal\n";
    }
}

cModule* SumoCombinedNodeManager::getNodeModule(const std::string& id)
{
    return getPersonNodeModule(id) == nullptr ? getVehicleNodeModule(id) : nullptr;
}

cModule* SumoCombinedNodeManager::getPersonNodeModule(const std::string& id)
{
    auto found = m_personNodes.find(id);
    return found != m_personNodes.end() ? found->second : nullptr;
}

cModule* SumoCombinedNodeManager::getVehicleNodeModule(const std::string& id)
{
    auto found = m_vehicleNodes.find(id);
    return found != m_vehicleNodes.end() ? found->second : nullptr;
}

void SumoCombinedNodeManager::eraseNode(const std::string& id){
    auto vfound = m_vehicleNodes.find(id);
    if (vfound != m_vehicleNodes.end()){
        m_vehicleNodes.erase(id);
    }
    auto pfound = m_personNodes.find(id);
    if (pfound != m_personNodes.end()){
        m_personNodes.erase(id);
    }
}

std::size_t SumoCombinedNodeManager::getNumberOfNodes() const
{
    return m_persons.size() + m_vehicles.size();
}

std::map<std::string, omnetpp::cModule*>& SumoCombinedNodeManager::getNodeVector(const std::string& vectorName){
    if (vectorName == m_person_module_vector){
        return m_personNodes;
    } else if (vectorName == m_vehicle_module_vector){
        return m_vehicleNodes;
    } else {
        throw cRuntimeError("wrong vector node name");
    }
}

VehicleSink* SumoCombinedNodeManager::getVehicleSink(cModule* node)
{
    ASSERT(node);
    cModule* module = node->getModuleByPath(m_vehicle_sink_module.c_str());
    if (!module) {
        throw cRuntimeError("No module found at %s relative to %s", m_vehicle_sink_module.c_str(), node->getFullPath().c_str());
    }

    auto* mobility = dynamic_cast<VehicleSink*>(module);
    if (!mobility) {
        throw cRuntimeError("Module %s is not a VehicleSink", module->getFullPath().c_str());
    }

    return mobility;
}

VehicleSink* SumoCombinedNodeManager::getVehicleSink(const std::string& id)
{
    auto found = m_vehicles.find(id);
    return found != m_vehicles.end() ? found->second : nullptr;
}

PersonSink* SumoCombinedNodeManager::getPersonSink(cModule* node)
{
    ASSERT(node);
    cModule* module = node->getModuleByPath(m_person_sink_module.c_str());
    if (!module) {
        throw cRuntimeError("No module found at %s relative to %s", m_person_sink_module.c_str(), node->getFullPath().c_str());
    }

    auto* mobility = dynamic_cast<PersonSink*>(module);
    if (!mobility) {
        throw cRuntimeError("Module %s is not a PersonSink", module->getFullPath().c_str());
    }

    return mobility;
}

PersonSink* SumoCombinedNodeManager::getPersonSink(const std::string& id)
{
    auto found = m_persons.find(id);
    return found != m_persons.end() ? found->second : nullptr;
}

void SumoCombinedNodeManager::visit(ITraciNodeVisitor *visitor) const{
    for (const auto& entry : m_personNodes) {
        visitor->visitNode(entry.first, entry.second);
    }
    for (const auto& entry : m_vehicleNodes) {
        visitor->visitNode(entry.first, entry.second);
    }
}


}
