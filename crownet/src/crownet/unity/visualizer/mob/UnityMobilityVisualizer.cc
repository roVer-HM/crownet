#include "crownet/unity/visualizer/mob/UnityMobilityVisualizer.h"

namespace crownet {

Define_Module(UnityMobilityVisualizer);

void UnityMobilityVisualizer::initialize(int stage) {
    MobilityVisualizerBase::initialize(stage);
    // instantly unsubscribe from module that is not supported
    MobilityVisualizerBase::visualizationSubjectModule->unsubscribe(PRE_MODEL_CHANGE, this);

    if (stage == inet::INITSTAGE_LOCAL) {
        unityClient = UnityClient::getInstance();
    }
}

void UnityMobilityVisualizer::receiveSignal(cComponent *source, inet::simsignal_t signal, cObject *object, cObject *details) {
    MobilityVisualizerBase::receiveSignal(source, signal, object, details);

    if (signal == inet::IMobility::mobilityStateChangedSignal && moduleFilter.matches(check_and_cast<cModule*>(source))) {
        sendMobilityUpdateMessage(source);
    }
}

void UnityMobilityVisualizer::sendMobilityUpdateMessage(cComponent* source) {
    if (auto* personMobility = dynamic_cast<artery::PersonMobility*>(source)) {
        unityClient->sendMessage(source->getFullPath(), "person", convertPositionToCoord(personMobility->getPosition()));
    } else if (auto* vehicleModule = dynamic_cast<artery::VehicleMobility*>(source)) {
        auto vehicleController = vehicleModule->getVehicleController();
        unityClient->sendMessage(source->getFullPath(), "vehicle", convertPositionToCoord(vehicleController->getPosition()));
    } else if (auto* stationaryMobility = dynamic_cast<inet::StationaryMobility*>(source)) {
        unityClient->sendMessage(source->getFullPath(), "stationary", stationaryMobility->getCurrentPosition());
    } else if (auto* vaderePersonMobility = dynamic_cast<InetVaderePersonMobility*>(source)) {
        unityClient->sendMessage(source->getFullPath(), "person", vaderePersonMobility->getCurrentPosition());
    } else {
        // Invalid mobility type
        std::cerr << "Invalid mobility type" << std::endl;
    }
}

UnityMobilityVisualizer::MobilityVisualization* UnityMobilityVisualizer::createMobilityVisualization(inet::IMobility *mobility) {
    return new MobilityVisualization(mobility);
}

inet::Coord UnityMobilityVisualizer::convertPositionToCoord(const artery::Position &pos) {
    return inet::Coord(pos.x.value(), pos.y.value(), 0.0);  // Assuming z-coordinate is not available
}

}  // namespace crownet
