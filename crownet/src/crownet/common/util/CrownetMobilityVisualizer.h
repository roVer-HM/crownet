#pragma once
#include "inet/visualizer/base/MobilityVisualizerBase.h"




namespace crownet{

class INET_API CrownetMobilityVisualizer : public inet::visualizer::MobilityVisualizerBase {
 public:
  virtual void initialize(int stage) override;
  virtual void receiveSignal(cComponent *source, inet::simsignal_t signal,
                             cObject *object, cObject *details) override;
  virtual void handleParameterChange(const char* name) override;
  virtual void subscribe() override;
  virtual void unsubscribe() override;
  virtual MobilityVisualization *createMobilityVisualization(inet::IMobility *mobility) override ;
  virtual void removeMobilityVisualization(const MobilityVisualization *visualization) override;

};


}
