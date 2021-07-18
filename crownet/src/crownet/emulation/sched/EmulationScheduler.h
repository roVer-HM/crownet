/*
 * EmulationScheduler.h
 *
 *  Created on: Jul 14, 2021
 *      Author: matthias
 */

#ifndef CROWNET_EMULATION_SCHED_EMULATIONSCHEDULER_H_
#define CROWNET_EMULATION_SCHED_EMULATIONSCHEDULER_H_

#include <inet/common/scheduler/RealTimeScheduler.h>

namespace crownet {

class EmulationScheduler : public inet::RealTimeScheduler {
public:
    virtual void startEmulation();

    // Needed for measurements
    virtual int64_t getBaseTime();
    virtual int64_t getElapsedRealtime();

};

}

#endif /* CROWNET_EMULATION_SCHED_EMULATIONSCHEDULER_H_ */
