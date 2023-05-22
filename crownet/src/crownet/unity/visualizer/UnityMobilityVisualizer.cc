#include "crownet/unity/visualizer/UnityMobilityVisualizer.h"

namespace crownet {

Define_Module(UnityMobilityVisualizer);

void UnityMobilityVisualizer::initialize(int stage) {
    MobilityVisualizerBase::initialize(stage);
    if (stage == inet::INITSTAGE_LOCAL) {

        // Get the UnityClient instance
        UnityClient *unityClient = UnityClient::getInstance();

        // Assign the instance to mobilityController
        mobilityController = unityClient;


    }

}

void UnityMobilityVisualizer::receiveSignal(cComponent *source,
        inet::simsignal_t signal, cObject *object, cObject *details) {
    MobilityVisualizerBase::receiveSignal(source, signal, object, details);

    if (signal == inet::IMobility::mobilityStateChangedSignal) {
        if (moduleFilter.matches(check_and_cast<cModule*>(source))) {
            artery::PersonMobility *personMobility =
                    dynamic_cast<artery::PersonMobility*>(source);
            artery::InetMobility *inetMobility =
                    dynamic_cast<artery::InetMobility*>(source);
            artery::VehicleMobility *vehicleModule =
                    dynamic_cast<artery::VehicleMobility*>(source);
            inet::StationaryMobility *stationaryMobility =
                    dynamic_cast<inet::StationaryMobility*>(source);
            InetVaderePersonMobility *vaderePersonMobility =
                    dynamic_cast<InetVaderePersonMobility*>(source);

            if (personMobility) {
                UnityClient::getInstance()->sendMessage(
                        personMobility->getPersonId(), "createOrUpdatePerson",
                        convertPositionToCoord(personMobility->getPosition()));
            } else if (vehicleModule) {
                auto vehicleController = vehicleModule->getVehicleController();
                UnityClient::getInstance()->sendMessage(
                        vehicleController->getVehicleId(),
                        "createOrUpdateVehicle",
                        convertPositionToCoord(
                                vehicleController->getPosition()));

            } else if (stationaryMobility) {

                UnityClient::getInstance()->sendMessage("stationary0",
                        "createStationary",
                        stationaryMobility->getCurrentPosition());

            }
            else if (vaderePersonMobility) {

                UnityClient::getInstance()->sendMessage(vaderePersonMobility->getPersonId(),"createOrUpdatePerson",vaderePersonMobility->getCurrentPosition());

                       }

            else {
                // None of the casts were successful
                std::cerr << "Invalid mobility type" << std::endl;
            }
        }

    }

}

// Add your custom signal handling code if needed
UnityMobilityVisualizer::MobilityVisualization*
UnityMobilityVisualizer::createMobilityVisualization(
        inet::IMobility *mobility) {
    auto visualization = new MobilityVisualization(mobility);
    return visualization;
}

inet::Coord UnityMobilityVisualizer::convertPositionToCoord(
        const artery::Position &pos) {
    // assuming x and y are of Position::value_type which has value() method
    double x = pos.x.value();
    double y = pos.y.value();
    double z = 0.0; // use 0 if not available

    return inet::Coord(x, y, z);
}

}  // namespace crownet
