/*
 * PedestrianMiddleware.cc
 *
 *  Created on: Aug 11, 2020
 *      Author: sts
 */

#include "PedestrianMiddleware.h"
#include <artery/application/MovingNodeDataProvider.h>
#include <artery/traci/PersonMobility.h>
#include <artery/utility/InitStages.h>
#include <vanetza/geonet/station_type.hpp>
#include "inet/common/ModuleAccess.h"

using namespace artery;

namespace crownet {

Define_Module(PedestrianMiddleware);

PedestrianMiddleware::~PedestrianMiddleware() {}

void PedestrianMiddleware::initialize(int stage) {
  using vanetza::geonet::StationType;

  if (stage == InitStages::Self) {
    findHost()->subscribe(MobilityBase::stateChangedSignal, this);
    initializeController(par("mobilityModule"));
    mDataProvider.setStationType(StationType::Pedestrian);
    setStationType(StationType::Pedestrian);
    getFacilities().register_const(&mDataProvider);
    // todo introduce PersonKinematics struct
    mDataProvider.update(getKinematics(*mPersonController));

    // emit will update mIdenity of Middelware base
    Identity identity;
    identity.traci = mPersonController->getTraciId();
    identity.application = mDataProvider.station_id();
    emit(Identity::changeSignal,
         Identity::ChangeTraCI | Identity::ChangeStationId, &identity);
  }
  Middleware::initialize(stage);
}

void PedestrianMiddleware::finish() {
  Middleware::finish();
  findHost()->unsubscribe(MobilityBase::stateChangedSignal, this);
}

void PedestrianMiddleware::initializeController(cPar& mobilityPar) {
  auto mobility =
      inet::getModuleFromPar<PersonMobility>(mobilityPar, findHost());

  if (!mobility) {
      error("Module not found on path '%s' defined by par '%s'",
                  mobilityPar.stringValue(), mobilityPar.getFullPath().c_str());
  }

  auto personController = mobility->getPersonController();

  mPersonController = dynamic_cast<VaderePersonController*>(personController);
  if(!mPersonController){
      error("Controller is not a valid VaderePersonController.");
  };

  ASSERT(mPersonController);
  getFacilities().register_mutable(mPersonController);
}

void PedestrianMiddleware::receiveSignal(cComponent* component,
                                         simsignal_t signal, cObject* obj,
                                         cObject* details) {
  if (signal == MobilityBase::stateChangedSignal) {
      // todo introduce PersonKinematics struct
    mDataProvider.update(getKinematics(*mPersonController));
  }
}

} /* namespace crownet */
