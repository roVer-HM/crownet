#include "artery/networking/Runtime.h"
#include "artery/traci/MobilityBase.h"
#include "artery/utility/InitStages.h"

#include "inet/common/ModuleAccess.h"

#include <vanetza/common/position_fix.hpp>
#include "crownet/artery/traci/PedestrianPositionProvider.h"

using namespace artery;

namespace crownet {

Define_Module(PedestrianPositionProvider)

    static const omnetpp::simsignal_t scPositionFixSignal =
        omnetpp::cComponent::registerSignal("PositionFix");

void PedestrianPositionProvider::initialize(int stage) {
  if (stage == artery::InitStages::Prepare) {
    mRuntime =
        inet::getModuleFromPar<artery::Runtime>(par("runtimeModule"), this);
    auto& mobilityPar = par("mobilityModule");
    auto* mobilityModule = getModuleByPath(mobilityPar);
    if (mobilityModule) {
      mobilityModule->subscribe(artery::MobilityBase::stateChangedSignal, this);
      if (auto mobilityBase =
              dynamic_cast<artery::MobilityBase*>(mobilityModule)) {
        mController = mobilityBase->getController<VaderePersonController>();
      } else {
        error("Module on path '%s' is not a MobilityBase",
              mobilityModule->getFullPath().c_str());
      }
    } else {
      error("Module not found on path '%s' defined by par '%s'",
            mobilityPar.stringValue(), mobilityPar.getFullPath().c_str());
    }
  } else if (stage == artery::InitStages::Propagate) {
    updatePosition();
  }
}

int PedestrianPositionProvider::numInitStages() const {
  return artery::InitStages::Total;
}

void PedestrianPositionProvider::receiveSignal(omnetpp::cComponent*,
                                               omnetpp::simsignal_t signal,
                                               omnetpp::cObject*,
                                               omnetpp::cObject*) {
  if (signal == MobilityBase::stateChangedSignal && mController) {
    updatePosition();
  }
}

void PedestrianPositionProvider::updatePosition() {
  using namespace vanetza::units;
  static const TrueNorth north;

  auto geopos = mController->getGeoPosition();
  mPositionFix.timestamp = mRuntime->now();
  mPositionFix.latitude = geopos.latitude;
  mPositionFix.longitude = geopos.longitude;
  mPositionFix.confidence.semi_minor = 5.0 * si::meter;
  mPositionFix.confidence.semi_major = 5.0 * si::meter;
  mPositionFix.course.assign(
      north + mController->getHeading().degree() * degree,
      north + 3.0 * degree);
  mPositionFix.speed.assign(mController->getSpeed(),
                            1.0 * si::meter_per_second);

  // prevent signal listeners to modify our position data
  PositionFixObject tmp{mPositionFix};
  emit(scPositionFixSignal, &tmp);
}

}  // namespace crownet
