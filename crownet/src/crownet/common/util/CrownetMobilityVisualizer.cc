#include "crownet/common/util/CrownetMobilityVisualizer.h"

namespace crownet {

Define_Module(CrownetMobilityVisualizer);

void CrownetMobilityVisualizer::initialize(int stage) {
  MobilityVisualizerBase::initialize(stage);
  if (stage == inet::INITSTAGE_LOCAL) {
      // initialize of other module in last step
    std::cout << "Test\n";
    par('foo')
    // Add your custom initialization code if needed
  }
}

void CrownetMobilityVisualizer::receiveSignal(cComponent *source,
                                              inet::simsignal_t signal,
                                              cObject *object,
                                              cObject *details) {
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

CrownetMobilityVisualizer::MobilityVisualizerBase::MobilityVisualization *
CrownetMobilityVisualizer::createMobilityVisualization(
    inet::IMobility *mobility) {
  auto visualization =
      new CrownetMobilityVisualizer::MobilityVisualization(mobility);
  return visualization;
}

void CrownetMobilityVisualizer::removeMobilityVisualization(
    const MobilityVisualization *visualization) {
  MobilityVisualizerBase::removeMobilityVisualization(visualization);
  // Add your custom remove MobilityVisualization code if needed
}
}  // namespace crownet
