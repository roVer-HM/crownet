/*
 * EmulationScheduler.cpp
 *
 *  Created on: Jul 14, 2021
 *      Author: matthias
 */

#include <omnetpp.h>
#include "EmulationScheduler.h"

Register_Class(crownet::EmulationScheduler);

int64_t crownet::EmulationScheduler::getBaseTime()
{
    return baseTime;
}

int64_t crownet::EmulationScheduler::getElapsedRealtime()
{
    return inet::opp_get_monotonic_clock_nsecs() - baseTime;
}

void crownet::EmulationScheduler::startEmulation()
{
    executionResumed();
}
