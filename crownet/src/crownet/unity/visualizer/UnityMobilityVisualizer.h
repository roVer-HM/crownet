#pragma once
#include "inet/visualizer/base/MobilityVisualizerBase.h"
#include "omnetpp/cobject.h"
#include <mutex>
#include "crownet/unity/controller/mobility/MobilityController.h"
#include "crownet/unity/client/UnityClient.h"

namespace crownet {

class UnityMobilityVisualizer: public inet::visualizer::MobilityVisualizerBase {
public:
    virtual void initialize(int stage) override;
    virtual void receiveSignal(cComponent *source, inet::simsignal_t signal,
            cObject *object, cObject *details) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void subscribe() override;
    virtual void unsubscribe() override;
    virtual MobilityVisualization* createMobilityVisualization(
            inet::IMobility *mobility) override;
    virtual void removeMobilityVisualization(
            const MobilityVisualization *visualization) override;

    cModule *module;
    UnityClient *mobilityController;
    std::mutex m_mutex;
    int test = 127;
};

}  // namespace crownet
