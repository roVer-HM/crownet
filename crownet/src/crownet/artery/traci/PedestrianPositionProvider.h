#pragma once

#include <omnetpp/clistener.h>
#include <omnetpp/csimplemodule.h>
#include <vanetza/common/position_provider.hpp>
#include "artery/networking/PositionFixObject.h"
#include "crownet/artery/traci/VaderePersonController.h"

namespace crownet {

class PedestrianPositionProvider : public omnetpp::cSimpleModule,
                                   public omnetpp::cListener,
                                   public vanetza::PositionProvider {
 public:
  // cSimpleModule
  void initialize(int stage) override;
  int numInitStages() const override;

  // cListener
  void receiveSignal(omnetpp::cComponent*, omnetpp::simsignal_t,
                     omnetpp::cObject*, omnetpp::cObject*) override;

  // PositionProvider
  const vanetza::PositionFix& position_fix() override { return mPositionFix; }

 private:
  void updatePosition();

  artery::PositionFixObject mPositionFix;
  artery::Runtime* mRuntime = nullptr;
  VaderePersonController* mController = nullptr;
};

}  // namespace crownet
