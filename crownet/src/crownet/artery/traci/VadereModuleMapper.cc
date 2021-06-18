/*
 * VadereModuleMapper.cc
 *
 *  Created on: Jun 14, 2021
 *      Author: vm-sts
 */

#include "VadereModuleMapper.h"
#include <omnetpp/ccomponenttype.h>
#include <omnetpp/distrib.h>
#include <omnetpp/cexception.h>

using namespace omnetpp;

namespace crownet {

Define_Module(VadereModuleMapper);

void VadereModuleMapper::initialize()
{
    m_rng = getRNG(0);
    m_person_type = cModuleType::find(par("personType"));

    m_person_penetration = par("personPenetrationRate");
}

cModuleType* VadereModuleMapper::person(traci::NodeManager& manager, const std::string& id)
{
    return (m_person_type && equipPerson()) ? m_person_type : nullptr;
}

cModuleType* VadereModuleMapper::vehicle(traci::NodeManager& manager, const std::string& id)
{
    throw new omnetpp::cRuntimeError("");
}

bool VadereModuleMapper::equipPerson()
{
    const double dice = omnetpp::uniform(m_rng, 0.0, 1.0);
    return dice < m_person_penetration;
}

} /* namespace crownet */
