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


namespace crownet {

class AppRxInfoProvider;
class NeighborhoodSizeProvider;

struct txInterval{
    simtime_t dInterval = -1.0;
    simtime_t rndInterval =  -1.0;
    simtime_t timestamp = -1.0;
    int pmembers = 1;
    b avg_pkt_size = b(0);
};

std::ostream& operator<<(std::ostream& os, const txInterval& i);



class DynamicMaxBandwidthScheduler: public IntervalScheduler {


public:
    DynamicMaxBandwidthScheduler();
    virtual ~DynamicMaxBandwidthScheduler();
    static simsignal_t txInterval_s;
    static simsignal_t txDetInterval_s;


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


    simtime_t last_tx;
    bool hasSent = false;
    bps appBandwidth;
    txInterval txIntervalDataPrio;
    txInterval txIntervalDataCurrent;

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
