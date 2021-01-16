/*
 * OsgCoordConverter.cc
 *
 *  Created on: Aug 24, 2020
 *      Author: sts
 */

#include "crownet/common/converter/OsgCoordConverter.h"

#include <omnetpp.h>
#include <memory>
#include "inet/common/INETDefs.h"
#include "inet/common/geometry/common/Coord.h"

namespace crownet {

Define_Module(OsgCoordConverter);

OsgCoordConverter::OsgCoordConverter() : _converter(nullptr) {}

void OsgCoordConverter::initialize(int stage) {
  cSimpleModule::initialize(stage);
  if (stage == inet::INITSTAGE_LOCAL) {
    if (hasPar("epgs_code")) {
      _converter = std::make_shared<OsgCoordinateConverter>(
          inet::Coord{par("offset_x").doubleValue(),
                      par("offset_y").doubleValue()},
          inet::Coord{par("xBound").doubleValue(), par("yBound").doubleValue()},
          par("epsg_code").stdstringValue());
    }
  }
}

std::shared_ptr<OsgCoordinateConverter> OsgCoordConverter::getConverter()
    const {
  return _converter;
}

bool OsgCoordConverter::isInitialized() const { return _converter != nullptr; }

void OsgCoordConverter::initializeConverter(
    std::shared_ptr<OsgCoordinateConverter> converter) {
  if (!_converter) {
    _converter = converter;
  } else {
    throw omnetpp::cRuntimeError(
        "OsgCoordianteTransformer already initialized.");
  }
}

void OsgCoordConverter::handleMessage(omnetpp::cMessage*) {
  throw omnetpp::cRuntimeError("OsgCoordConverter does not handle messages");
}

} /* namespace crownet */
