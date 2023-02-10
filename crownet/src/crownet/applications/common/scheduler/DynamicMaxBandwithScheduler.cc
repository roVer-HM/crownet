/*
 * DynamicMaxBandwithScheduler.cc
 *
 *  Created on: Feb 7, 2023
 *      Author: vm-sts
 */

#include "crownet/crownet.h"
#include "crownet/applications/common/scheduler/DynamicMaxBandwithScheduler.h"
#include "crownet/applications/common/info/AppRxInfoProvider.h"
#include "crownet/applications/common/info/AppRxInfo.h"

using namespace inet;

using namespace crownet::queueing;

namespace crownet {

std::ostream& operator<<(std::ostream& os, const txInterval& i){
    os << "(T_det: " << i.dInterval.ustr() << ", T_rnd: " << i.rndInterval.ustr() \
            << ", ts: " << i.timestamp.ustr() << ", avg_pkt_size: " \
            << i.avg_pkt_size << ", pmember: " << i.pmembers << ")";
    return os;
}

simsignal_t DynamicMaxBandwithScheduler::txInterval_s = cComponent::registerSignal("txInterval_s");
simsignal_t DynamicMaxBandwithScheduler::txDetInterval_s = cComponent::registerSignal("txDetInterval_s");

Define_Module(DynamicMaxBandwithScheduler)

DynamicMaxBandwithScheduler::DynamicMaxBandwithScheduler() {
    // TODO Auto-generated constructor stub

}

DynamicMaxBandwithScheduler::~DynamicMaxBandwithScheduler() {
    cancelClockEvent(generationTimer);
}

void DynamicMaxBandwithScheduler::initialize(int stage)
{
    IntervalScheduler::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        minTransmissionIntervall = par("generationInterval");
        maxApplicationBandwith = bps(par("maxApplicationBandwith").doubleValueInUnit("bps"));
        rndIntervalLowerBound = par("rndIntervalLowerBound").doubleValue();
        rndIntervalUpperBound = par("rndIntervalUpperBound").doubleValue();
        estimatedAvgPaketSize = b(par("estimatedAvgPaketSize"));
        appRxInfoProvider = getModuleFromPar<AppRxInfoProvider>(par("appRxInfoProviderModule"), this);

        last_tx = 0.0;
        hasSent = false;
        appBandwidth = maxApplicationBandwith;

        WATCH(txIntervalDataPrio);
        WATCH(txIntervalDataCurrent);
    }
}

void DynamicMaxBandwithScheduler::scheduleGenerationTimer(){

    if (txIntervalDataPrio.timestamp < 0.0){
        // first scheduling
        txIntervalDataPrio.avg_pkt_size = estimatedAvgPaketSize;
        txIntervalDataPrio.pmembers = 1;
        txIntervalDataPrio.timestamp = simTime();
        computeInterval(txIntervalDataPrio);
        EV_INFO << "first  scheduling using: " << txIntervalDataPrio << endl;
        scheduleClockEventAfter(SIMTIME_AS_CLOCKTIME(txIntervalDataPrio.rndInterval), generationTimer);
    } else {
        throw cRuntimeError("DynamicMaxBandwithScheduler is scheduled in handleMessage. Only use scheduleGenerationTimer at startup");
    }
}

void DynamicMaxBandwithScheduler::handleMessage(cMessage *message){
    auto now = simTime();
    if (message == generationTimer){
        updateTxIntervalDataCurrent();
        EV_INFO << LOG_MOD << "Prio interval: " << txIntervalDataPrio << endl;
        EV_INFO << LOG_MOD << "Current interval: " << txIntervalDataCurrent << endl;
        auto updatedTxTime = txIntervalDataPrio.timestamp + txIntervalDataCurrent.rndInterval;
        if (updatedTxTime < now){
            EV_INFO << LOG_MOD << "Transmit now" << endl;
            // sent data now because updated transmission time lies in the past
            scheduleApp(message);
            hasSent = true;
            emit(txInterval_s, simTime() - last_tx);

            last_tx = simTime();
            // overide prio txIntervalDataPrio with current interval
            txIntervalDataPrio = txIntervalDataCurrent;
            EV_INFO << LOG_MOD << "Schedule next transmission attempt at: " << (simTime() + txIntervalDataPrio.rndInterval).ustr()  << endl;
            scheduleClockEventAfter(SIMTIME_AS_CLOCKTIME(txIntervalDataPrio.rndInterval), generationTimer);
        } else {
            // wait with transmission to next updatedTxTime. Do not update txIntervalData yet.
            EV_INFO << LOG_MOD << "Delay transmission until " << updatedTxTime.ustr() << endl;
            scheduleClockEventAt(SIMTIME_AS_CLOCKTIME(updatedTxTime), generationTimer);
        }
    } else{
       throw cRuntimeError("Unknown message");
    }
    updateDisplayString();
}

void DynamicMaxBandwithScheduler::updateTxIntervalDataCurrent(){
    auto now = simTime();
    if (txIntervalDataCurrent.timestamp >= now){
        /*do nothing already updated*/
        return;
    }
    txIntervalDataCurrent.timestamp = now;
    // ensure to count at least one self.
    txIntervalDataCurrent.pmembers = std::max(1, appRxInfoProvider->getNumberOfSenders());
    txIntervalDataCurrent.avg_pkt_size = appRxInfoProvider->getAppRxInfo()->getAvg_packet_size();
    computeInterval(txIntervalDataCurrent);

}

void DynamicMaxBandwithScheduler::computeInterval(txInterval& tx){
    simtime_t C = (tx.avg_pkt_size / appBandwidth).get(); // b/bps --> s
    tx.dInterval = std::max(getMinTransmissionInterval(), tx.pmembers*C);
    // RFC 3550 p.30 (e-1.5)
    tx.rndInterval = rndInterval(tx.dInterval)/1.2182;
}

simtime_t DynamicMaxBandwithScheduler::getMinTransmissionInterval() const{
    return (hasSent) ? minTransmissionIntervall : minTransmissionIntervall/2;
}

simtime_t DynamicMaxBandwithScheduler::rndInterval(simtime_t dInterval){
    return uniform(rndIntervalLowerBound*dInterval, rndIntervalUpperBound*dInterval);
}



} /* namespace crownet */