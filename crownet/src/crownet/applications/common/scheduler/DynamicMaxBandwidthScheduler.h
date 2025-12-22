/*
 * DynamicMaxBandwidthScheduler.h
 *
 *  Created on: Feb 7, 2023
 *      Author: vm-sts
 */

#ifndef CROWNET_APPLICATIONS_COMMON_SCHEDULER_DYNAMICMAXBANDWIDTHSCHEDULER_H_
#define CROWNET_APPLICATIONS_COMMON_SCHEDULER_DYNAMICMAXBANDWIDTHSCHEDULER_H_

#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/common/Units.h"

#include "crownet/applications/common/scheduler/IntervalScheduler.h"

using namespace inet;
using namespace crownet::queueing;

/**
 * Correction factor to adjust the randomized interval to achieve the desired average sending rate.
 *
 * RFC 3550 p.30 (e-1.5)
 */
#define ARCDSA_CORRECTION_FACTOR (1.0 / (M_E - 1.5))

namespace crownet {

class AppRxInfoProvider;
class NeighborhoodSizeProvider;

struct txInterval{
    simtime_t dInterval = -1.0;
    simtime_t rndInterval =  -1.0;
    simtime_t timestamp = -1.0;
    simtime_t lastTransmisionInterval = -1.0;
    int pmembers = 1;
    double avg_pkt_size = 0.0; // unit: b (cannot use inet::b due to required double precision)
};

std::ostream& operator<<(std::ostream& os, const txInterval& i);



class DynamicMaxBandwidthScheduler: public IntervalScheduler {


public:
    DynamicMaxBandwidthScheduler();
    virtual ~DynamicMaxBandwidthScheduler();
    static simsignal_t txInterval_s;
    static simsignal_t txDetInterval_s;
    static simsignal_t txMemberValue_s;


protected:
    virtual void initialize(int stage) override;
    // handle IAppScheduler without any additional logic
    virtual void handleMessage(cMessage *message) override;

    virtual void scheduleGenerationTimer() override;

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t i, cObject *details) override;

    simtime_t getMinTransmissionInterval() const;
    void updateTxIntervalDataCurrent();
    void computeInterval(txInterval& tx);
    simtime_t rndInterval(simtime_t dInterval );

    virtual Unit getScheduledUnit() override;

    simtime_t getTimeSinceLastTransmission() const;


    simtime_t last_tx;
    bool hasSent = false;
    bps appBandwidth;
    txInterval txIntervalDataPrior;     // tx interval calculated for the next transmission
    txInterval txIntervalDataCurrent;  // tx interval calculated at the current point in time

    simtime_t minTransmissionIntervall;
    bps maxApplicationBandwidth;
    b estimatedAvgPaketSize;
    AppRxInfoProvider* appRxInfoProvider = nullptr;

    NeighborhoodSizeProvider* ntSizeProvider = nullptr;

    double rndIntervalLowerBound;
    double rndIntervalUpperBound;

private:
    bool isFirstScheduleCall = true;


};

} /* namespace crownet */

#endif /* CROWNET_APPLICATIONS_COMMON_SCHEDULER_DYNAMICMAXBANDWIDTHSCHEDULER_H_ */
