//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "PedestrianMiddelwareEmpyt.h"
#include <artery/application/MovingNodeDataProvider.h>
#include <artery/traci/MobilityBase.h>
#include <artery/utility/InitStages.h>
#include <vanetza/geonet/station_type.hpp>
#include "inet/common/ModuleAccess.h"

using namespace artery;

namespace rover {

Define_Module(PedestrianMiddelwareEmpyt);

void PedestrianMiddelwareEmpyt::initialize(int stage) {
  using vanetza::geonet::StationType;
  MiddlewareBase::initialize(stage);

  if (stage == InitStages::Self) {
    findHost()->subscribe(MobilityBase::stateChangedSignal, this);
    initializeController(par("mobilityModule"));
    mDataProvider.setStationType(StationType::Pedestrian);
    getFacilities().register_const(&mDataProvider);
    mDataProvider.update(mPersonController);

    Identity identity;
    identity.traci = mPersonController->getNodeId();
    identity.application = mDataProvider.station_id();
    emit(Identity::changeSignal,
         Identity::ChangeTraCI | Identity::ChangeStationId, &identity);
  }
}

void PedestrianMiddelwareEmpyt::finish() {
  MiddlewareBase::finish();
  findHost()->unsubscribe(MobilityBase::stateChangedSignal, this);
}

void PedestrianMiddelwareEmpyt::initializeController(cPar& mobilityPar) {
  auto mobility =
      inet::getModuleFromPar<ControllableObject>(mobilityPar, findHost());
  mPersonController = mobility->getController<VaderePersonController>();
  ASSERT(mPersonController);
  getFacilities().register_mutable(mPersonController);
}

void PedestrianMiddelwareEmpyt::receiveSignal(cComponent* component,
                                              simsignal_t signal, cObject* obj,
                                              cObject* details) {
  if (signal == MobilityBase::stateChangedSignal) {
    mDataProvider.update(mPersonController);
  }
}

}  // namespace rover
