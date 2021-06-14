/*
 * OsgCoordConverter.cc
 *
 *  Created on: Aug 24, 2020
 *      Author: sts
 */

#include "crownet/common/converter/OsgCoordConverter.h"

#include <omnetpp.h>
#include <memory>
#include <traci/Core.h>
#include <traci/Position.h>
#include <inet/common/ModuleAccess.h>
#include "inet/common/INETDefs.h"
#include "inet/common/geometry/common/Coord.h"
#include "crownet/artery/traci/VadereApi.h"
#include "crownet/artery/traci/VadereCore.h"

namespace crownet {

Define_Module(OsgCoordConverterLocal);
Define_Module(OsgCoordConverterVadere);
Define_Module(OsgCoordConverterSumo);

void OsgCoordConverterLocal::initialize(int stage) {
  cSimpleModule::initialize(stage);
  if (stage == inet::INITSTAGE_LOCAL) {
      _converter = std::make_shared<OsgCoordinateConverter>(
          inet::Coord{par("offset_x").doubleValue(),
                      par("offset_y").doubleValue()},
          inet::Coord{par("xBound").doubleValue(), par("yBound").doubleValue()},
                      par("epsg_code").stdstringValue());
    }
}

void OsgCoordConverterLocal::handleMessage(omnetpp::cMessage*) {
  throw omnetpp::cRuntimeError("OsgCoordConverter does not handle messages");
}

////////////////////////////////////////////


void OsgCoordConverterVadere::initialize(int stage) {
    cSimpleModule::initialize(stage);
  if (stage == inet::INITSTAGE_LOCAL) {

  } else if (stage == inet::INITSTAGE_LAST){
      VadereCore* core = inet::getModuleFromPar<VadereCore>(par("coreModule"), this);
      subscribeTraCI(core);
  }
}

void OsgCoordConverterVadere::traciConnected(){
    VadereCore* core =
                inet::getModuleFromPar<VadereCore>(par("coreModule"), this);
    auto m_api = core->getVadereApi();
    CoordRef ref = m_api->v_simulation.getCoordRef();
    traci::Boundary netBound = traci::Boundary(m_api->v_simulation.getNetBoundary());
    _converter = std::make_shared<OsgCoordinateConverter>(
              ref.offset, netBound, ref.epsg_code);
    m_api->setConverter(_converter);

}

void OsgCoordConverterVadere::handleMessage(omnetpp::cMessage*) {
  throw omnetpp::cRuntimeError("OsgCoordConverterVadere does not handle messages");
}

////////////////////////////////////////////

void OsgCoordConverterSumo::initialize(int stage) {
    cSimpleModule::initialize(stage);
  if (stage == inet::INITSTAGE_LOCAL) {

  } else if (stage == inet::INITSTAGE_LAST){
      Core* core = inet::getModuleFromPar<Core>(par("coreModule"), this);
      subscribeTraCI(core);
  }
}

void OsgCoordConverterSumo::traciConnected(){
    Core* core =
                inet::getModuleFromPar<Core>(par("coreModule"), this);
    auto api = core->getAPI();
    traci::Boundary netBoundary =
              traci::Boundary(api->simulation.getNetBoundary());
    traci::TraCIPosition offset = traci::TraCIPosition(0, 0);
    _converter = std::make_shared<OsgCoordinateConverter>(
             offset, netBoundary, par("epsg_code").stdstringValue());
}

void OsgCoordConverterSumo::handleMessage(omnetpp::cMessage*) {
  throw omnetpp::cRuntimeError("OsgCoordConverterSumo does not handle messages");
}

////////////////////////////////////////////

} /* namespace crownet */
