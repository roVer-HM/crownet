#include "crownet/unity/visualizer/CrownetMobilityVisualizer.h"

namespace crownet {

Define_Module(CrownetMobilityVisualizer);

void CrownetMobilityVisualizer::initialize(int stage) {
  MobilityVisualizerBase::initialize(stage);
  if (stage == inet::INITSTAGE_LOCAL) {
    EV << "Test";
    // Add your custom initialization code if needed
  }
}

void CrownetMobilityVisualizer::receiveSignal(cComponent *source,
                                              inet::simsignal_t signal,
                                              cObject *object,
                                              cObject *details) {
  if (signal == inet::IMobility::mobilityStateChangedSignal) {
    if (moduleFilter.matches(check_and_cast<cModule *>(source))) {
      auto mobility = dynamic_cast<inet::IMobility *>(source);
      auto mobilityVisualization = getMobilityVisualization(mobility);
      if (mobilityVisualization == nullptr) {
        mobilityVisualization = createMobilityVisualization(mobility);
        addMobilityVisualization(mobility, mobilityVisualization);
      }
      EV << mobility->getCurrentPosition();
      EV << mobility->getId();
    }
  }

  MobilityVisualizerBase::receiveSignal(source, signal, object, details);
  // Add your custom signal handling code if needed
}

void CrownetMobilityVisualizer::handleParameterChange(const char *name) {
  MobilityVisualizerBase::handleParameterChange(name);
  // Add your custom parameter handling code if needed
}

void CrownetMobilityVisualizer::subscribe() {
  MobilityVisualizerBase::subscribe();
  // Add your custom subscription code if needed
}

void CrownetMobilityVisualizer::unsubscribe() {
  MobilityVisualizerBase::unsubscribe();
  // Add your custom unsubscription code if needed
}

CrownetMobilityVisualizer::MobilityVisualization *
CrownetMobilityVisualizer::createMobilityVisualization(
    inet::IMobility *mobility) {
  auto visualization = new MobilityVisualization(mobility);
  return visualization;
}

void CrownetMobilityVisualizer::removeMobilityVisualization(
    const MobilityVisualization *visualization) {
  MobilityVisualizerBase::removeMobilityVisualization(visualization);
  // Add your custom remove MobilityVisualization code if needed
}
}  // namespace crownet
