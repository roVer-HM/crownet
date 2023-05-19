#include "crownet/unity/visualizer/UnityMobilityVisualizer.h"
#include <numeric>

namespace crownet {

Define_Module(UnityMobilityVisualizer);

void UnityMobilityVisualizer::initialize(int stage) {
    MobilityVisualizerBase::initialize(stage);
    if (stage == inet::INITSTAGE_LOCAL) {

            // Get the UnityClient instance
            UnityClient* unityClient = UnityClient::getInstance();

            // Assign the instance to mobilityController
         mobilityController = unityClient;

        //module = this->getSubmodule("mobilityController");
        //mobilityController = check_and_cast<MobilityController*>(module);

        //module = this->getSubmodule("unityClient");
        //mobilityController = check_and_cast<UnityClient*>(UnityClient::getInstance());

    }

}

void UnityMobilityVisualizer::receiveSignal(cComponent *source,
        inet::simsignal_t signal, cObject *object, cObject *details) {
    if (signal == inet::IMobility::mobilityStateChangedSignal) {
        if (moduleFilter.matches(check_and_cast<cModule*>(source))) {
            auto mobility = dynamic_cast<inet::IMobility*>(source);
            auto mobilityVisualization = getMobilityVisualization(mobility);
            if (mobilityVisualization == nullptr) {
                mobilityVisualization = createMobilityVisualization(mobility);
                addMobilityVisualization(mobility, mobilityVisualization);
            }



UnityClient::getInstance()->sendMessage(0, mobility->getCurrentPosition());
                //std::cout << mobilityController->getId()  << std::endl;
                //mobilityController->sendMessage(0, mobility->getCurrentPosition());

                //mobilityController->sendPosition(mobility->getId(),mobility->getCurrentPosition());



        }
    }

    MobilityVisualizerBase::receiveSignal(source, signal, object, details);
    // Add your custom signal handling code if needed
}

void UnityMobilityVisualizer::handleParameterChange(const char *name) {
    MobilityVisualizerBase::handleParameterChange(name);
    // Add your custom parameter handling code if needed
}

void UnityMobilityVisualizer::subscribe() {
    MobilityVisualizerBase::subscribe();
    // Add your custom subscription code if needed
}

void UnityMobilityVisualizer::unsubscribe() {
    MobilityVisualizerBase::unsubscribe();
    // Add your custom unsubscription code if needed
}

UnityMobilityVisualizer::MobilityVisualization*
UnityMobilityVisualizer::createMobilityVisualization(inet::IMobility *mobility) {
    auto visualization = new MobilityVisualization(mobility);
    return visualization;
}

void UnityMobilityVisualizer::removeMobilityVisualization(
        const MobilityVisualization *visualization) {
    MobilityVisualizerBase::removeMobilityVisualization(visualization);
    // Add your custom remove MobilityVisualization code if needed
}
}  // namespace crownet
