/*
 * VaderePersonSink.h
 *
 *  Created on: Jun 11, 2021
 *      Author: vm-sts
 */

#pragma once

#include <memory>
#include <traci/Boundary.h>
#include <traci/Position.h>
#include <traci/Angle.h>

using namespace traci;

namespace crownet {

class VadereApi;
class VaderePersonCache;

class VaderePersonSink
{
public:
    virtual void initializeSink(std::shared_ptr<API>, std::shared_ptr<PersonCache>, const Boundary&) = 0;
    virtual void initializePerson(const TraCIPosition&, TraCIAngle, double speed) = 0;
    virtual void updatePerson(const TraCIPosition&, TraCIAngle, double speed) = 0;
    virtual ~VaderePersonSink() = default;
};


}
