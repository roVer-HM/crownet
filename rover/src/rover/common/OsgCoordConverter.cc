/*
 * OsgCoordConverter.cc
 *
 *  Created on: Aug 24, 2020
 *      Author: sts
 */

#include "OsgCoordConverter.h"
#include <omnetpp.h>
#include <memory>
#include "inet/common/INETDefs.h"
#include "inet/common/geometry/common/Coord.h"

namespace rover {

Define_Module(OsgCoordConverter);

OsgCoordConverter::OsgCoordConverter() : _transformer(nullptr) {}

void OsgCoordConverter::initialize(int stage) {
  cSimpleModule::initialize(stage);
  if (stage == inet::INITSTAGE_LOCAL) {
    if (hasPar("epgs_code")) {
      _transformer = std::make_shared<OsgCoordianteTransformer>(
          par("epsg_code").stdstringValue(),
          inet::Coord{par("offset_x").doubleValue(),
                      par("offset_y").doubleValue()});
    }
  }
}

std::shared_ptr<OsgCoordianteTransformer> OsgCoordConverter::getTransformer()
    const {
  return _transformer;
}

bool OsgCoordConverter::isInitialized() const {
  return _transformer != nullptr;
}

void OsgCoordConverter::initializeTransformer(
    std::shared_ptr<OsgCoordianteTransformer> transformer) {
  if (!_transformer) {
    _transformer = transformer;
  } else {
    throw omnetpp::cRuntimeError(
        "OsgCoordianteTransformer already initialized.");
  }
}

void OsgCoordConverter::handleMessage(omnetpp::cMessage*) {
  throw omnetpp::cRuntimeError("OsgCoordConverter does not handle messages");
}

} /* namespace rover */
