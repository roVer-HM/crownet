/*
 * VadereModuleMapper.h
 *
 *  Created on: Jun 14, 2021
 *      Author: vm-sts
 */

#pragma once

#include <traci/ModuleMapper.h>
#include <omnetpp/csimplemodule.h>
#include <omnetpp/crng.h>

namespace crownet {

class VadereModuleMapper : public traci::ModuleMapper, public omnetpp::cSimpleModule {
public:
    virtual ~VadereModuleMapper() = default;

    void initialize() override;
    omnetpp::cModuleType* vehicle(traci::NodeManager&, const std::string&) override;
    omnetpp::cModuleType* person(traci::NodeManager&, const std::string&) override;
protected:
    virtual bool equipPerson();
private:
    omnetpp::cModuleType* m_person_type;
    omnetpp::cRNG* m_rng;
    double m_person_penetration;
};

} /* namespace crownet */

