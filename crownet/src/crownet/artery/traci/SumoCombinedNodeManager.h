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

#pragma once

#include <traci/BasicNodeManager.h>
#include <traci/Listener.h>
#include <traci/ModuleMapper.h>
#include <traci/NodeManager.h>
#include "traci/Position.h"
#include "traci/SubscriptionManager.h"
//#include "traci/PersonSink.h"

namespace traci {
class API;
class ModuleMapper;
class PersonSink;
class VehicleCache;
class VehicleSink;
}

using namespace traci;
namespace  crownet {

class SumoCombinedNodeManager : public traci::NodeManager,
                                public traci::Listener,
                                public omnetpp::cSimpleModule {
public:
    static const omnetpp::simsignal_t addNodeSignal;
    static const omnetpp::simsignal_t updateNodeSignal;
    static const omnetpp::simsignal_t removeNodeSignal;
    static const omnetpp::simsignal_t addPersonSignal;
    static const omnetpp::simsignal_t updatePersonSignal;
    static const omnetpp::simsignal_t removePersonSignal;
    static const omnetpp::simsignal_t addVehicleSignal;
    static const omnetpp::simsignal_t updateVehicleSignal;
    static const omnetpp::simsignal_t removeVehicleSignal;

    std::shared_ptr<API> getAPI() override { return m_api; }
    SubscriptionManager* getSubscriptions() { return m_subscriptions; }
    std::size_t getNumberOfNodes() const override;

    SumoCombinedNodeManager();
    virtual ~SumoCombinedNodeManager();
    void initialize() override;

    /**
     * VehicleObject wraps variable cache of a subscribed TraCI vehicle
     *
     * Each emitted vehicle update signal is accompanied by a VehicleObject (cObject details)
     */
    class VehicleObject : public NodeManager::MovingObject
    {
    public:
        virtual std::shared_ptr<VehicleCache> getCache() const = 0;
    };

    class PersonObject : public NodeManager::MovingObject
    {
    public:
        virtual std::shared_ptr<PersonCache> getCache() const = 0;
    };

protected:
    using NodeInitializer = std::function<void(omnetpp::cModule*)>;

    void finish() override;

    void traciInit() override;
    void traciStep() override;
    void traciClose() override;

    virtual void addPerson(const std::string&);
    virtual void removePerson(const std::string&);
    virtual void updatePerson(const std::string&, PersonSink*);
    virtual void addVehicle(const std::string&);
    virtual void removeVehicle(const std::string&);
    virtual void updateVehicle(const std::string&, VehicleSink*);
    virtual omnetpp::cModule* createModule(const std::string&, omnetpp::cModuleType*, const std::string& moduleVector);
    virtual omnetpp::cModule* addNodeModule(const std::string&, omnetpp::cModuleType*, NodeInitializer&, const std::string& moduleVector);
    virtual void removePersonNodeModule(const std::string&);
    virtual void removeVehicleNodeModule(const std::string&);

    virtual omnetpp::cModule* getNodeModule(const std::string&);
    virtual omnetpp::cModule* getPersonNodeModule(const std::string& id);
    virtual omnetpp::cModule* getVehicleNodeModule(const std::string& id);
    virtual PersonSink* getPersonSink(omnetpp::cModule*);
    virtual PersonSink* getPersonSink(const std::string&);
    virtual VehicleSink* getVehicleSink(omnetpp::cModule*);
    virtual VehicleSink* getVehicleSink(const std::string&);
    virtual void processPersons();
    virtual void processVehicles();

private:
    virtual void eraseNode(const std::string& id);
    virtual std::map<std::string, omnetpp::cModule*>& getNodeVector(const std::string& vectorName);

protected:
    std::shared_ptr<API> m_api;
    ModuleMapper* m_mapper;
    Boundary m_boundary;
    SubscriptionManager* m_subscriptions;

    // person
    std::map<std::string, PersonSink*> m_persons;
    std::map<std::string, omnetpp::cModule*> m_personNodes;
    std::string m_person_sink_module;
    std::string m_person_module_vector;
    bool m_ignore_persons;

    // vehicle
    std::map<std::string, VehicleSink*> m_vehicles;
    std::map<std::string, omnetpp::cModule*> m_vehicleNodes;
    std::string m_vehicle_sink_module;
    std::string m_vehicle_module_vector;
    bool m_ignore_vehicle;
    bool m_destroy_vehicles_on_crash;

    omnetpp::SimTime m_offset = omnetpp::SimTime::ZERO;
    std::map<std::string, unsigned> m_nodeIndex;

};

}
