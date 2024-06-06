#pragma once
#include "inet/visualizer/base/MobilityVisualizerBase.h"
#include "omnetpp/cobject.h"
#include <mutex>
#include "crownet/unity/client/UnityClient.h"
#include "artery/traci/PersonMobility.h"
#include "artery/traci/VehicleMobility.h"
#include "inet/mobility/static/StationaryMobility.h"
#include "artery/traci/PersonMobility.h"
#include "artery/inet/InetMobility.h"
#include "artery/traci/VehicleMobility.h"
#include "crownet/artery/traci/InetVaderePersonMobility.h"

namespace crownet {

class UnityMobilityVisualizer : public inet::visualizer::MobilityVisualizerBase {
public:
    UnityMobilityVisualizer() = default;
    virtual ~UnityMobilityVisualizer() = default;

    void initialize(int stage) override;
    void receiveSignal(cComponent *source, inet::simsignal_t signal, cObject *object, cObject *details) override;

    MobilityVisualization* createMobilityVisualization(inet::IMobility *mobility) override;

protected:
    inet::Coord convertPositionToCoord(const artery::Position& pos);

private:
    void sendMobilityUpdateMessage(cComponent* source);

    UnityClient *unityClient;
    std::mutex m_mutex;
};

}  // namespace crownet
