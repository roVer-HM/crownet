#pragma once
#include "inet/visualizer/base/MobilityVisualizerBase.h"
#include "omnetpp/cobject.h"
#include <mutex>
#include "crownet/unity/controller/mobility/MobilityController.h"
#include "crownet/unity/client/UnityClient.h"
#include <numeric>
#include "artery/traci/PersonMobility.h"
#include "artery/traci/VehicleMobility.h"
#include "inet/mobility/static/StationaryMobility.h"
#include "artery/traci/PersonMobility.h"
#include "artery/inet/InetMobility.h"
#include "artery/traci/VehicleMobility.h"
#include "crownet/artery/traci/InetVaderePersonMobility.h"


namespace crownet {

class UnityMobilityVisualizer: public inet::visualizer::MobilityVisualizerBase {
public:    virtual void initialize(int stage) override;
    virtual void receiveSignal(cComponent *source, inet::simsignal_t signal,
            cObject *object, cObject *details) override;
    virtual void handleParameterChange(const char *name) override {
        inet::visualizer::MobilityVisualizerBase::handleParameterChange(name);
    };
    virtual void subscribe() override{
        inet::visualizer::MobilityVisualizerBase::subscribe();
    };
    virtual void unsubscribe() override{
        inet::visualizer::MobilityVisualizerBase::unsubscribe();
            };


    virtual void removeMobilityVisualization(
            const MobilityVisualization *visualization) override {
        inet::visualizer::MobilityVisualizerBase::removeMobilityVisualization(visualization);

    }

    virtual MobilityVisualization* createMobilityVisualization(
            inet::IMobility *mobility) override;


    cModule *module;
    UnityClient *mobilityController;
    std::mutex m_mutex;

protected:
inet::Coord convertPositionToCoord(const artery::Position& pos);

};


}  // namespace crownet
