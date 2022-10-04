/*
 * OsgCoordConverter.cc
 *
 *  Created on: Aug 24, 2020
 *      Author: sts
 */

#include "crownet/common/converter/OsgCoordConverter.h"

#include <omnetpp.h>
#include <memory>
#include <algorithm>
#include <traci/Core.h>
#include <traci/Position.h>
#include <inet/common/ModuleAccess.h>
#include "inet/common/INETDefs.h"
#include "inet/common/geometry/common/Coord.h"
#include "crownet/artery/traci/VadereApi.h"
#include "crownet/artery/traci/VadereUtils.h"
#include "crownet/artery/traci/VadereCore.h"
#include "crownet/common/util/crownet_util.h"

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


namespace {

const simsignal_t simBoundSignal = cComponent::registerSignal("simBoundSig");
const simsignal_t simOffsetSignal = cComponent::registerSignal("simOffsetSig");

}

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
                      par("srs_code").stdstringValue());
      _converter->setCellSize(par("cellSize").doubleValue());
      auto areaOfIntrest = dynamic_cast<AreaOfInterest*>(par("areaOfInterest").objectValue());
      _converter->setAreaOfInterest(areaOfIntrest);
      emit(simBoundSignal, this);
      emit(simOffsetSignal, this);
    }
}

inet::Coord OsgCoordConverterLocal::computeSceneCoordinate(const inet::GeoCoord& geographicCoordinate) const{
    return _converter->convert2D(geographicCoordinate, true);
}

inet::GeoCoord OsgCoordConverterLocal::computeGeographicCoordinate(const inet::Coord& sceneCoordinate) const {
    return _converter->convertToGeoInet(sceneCoordinate);
}


void OsgCoordConverterLocal::handleMessage(omnetpp::cMessage*) {
  throw omnetpp::cRuntimeError("OsgCoordConverter does not handle messages");
}

////////////////////////////////////////////


void OsgCoordConverterVadere::initialize(int stage) {
  cSimpleModule::initialize(stage);
  if (stage == inet::INITSTAGE_LOCAL) {
      // use current directory as fallback to create absolute paths for Vadere.
      fs::path iniBaseDir = fs::current_path();
      fs::path resultDir (cSimulation::getActiveSimulation()
                                                        ->getEnvir()
                                                        ->getConfigEx()
                                                        ->getVariable(CFGVAR_RESULTDIR));
      resultDir = resultDir.is_relative() ? fs::absolute(resultDir, iniBaseDir) : resultDir;

      fs::path vSPath (par("vadereScenarioPath").stdstringValue());
      vSPath = vSPath.is_relative() ? fs::absolute(vSPath, iniBaseDir) : vSPath;

      vadere::CoordianteSystemSettings settings =
              vadere::getCoordianteSystemSettings(vSPath.string());

      CoordRef ref;
      ref.epsg_code = settings.epsg;
      // offset needs to be negative offset to mirror sumo behavior
      ref.offset = TraCIPosition(-1*settings.offsetX, -1*settings.offsetY);
      traci::Boundary netBound(settings.bound);

      _converter = std::make_shared<OsgCoordinateConverter>(
                ref.offset, netBound, ref.epsg_code);
      _converter->setCellSize(par("cellSize").doubleValue());
      auto areaOfIntrest = dynamic_cast<AreaOfInterest*>(par("areaOfIntrest").objectValue());
      _converter->setAreaOfInterest(areaOfIntrest);

      emit(simBoundSignal, this);
      emit(simOffsetSignal, this);
  } else if (stage == inet::INITSTAGE_LAST){
      VadereCore* core = inet::getModuleFromPar<VadereCore>(par("coreModule"), this);
      subscribeTraCI(core);
  }

}

void OsgCoordConverterVadere::traciConnected(){
    VadereCore* core =
                inet::getModuleFromPar<VadereCore>(par("coreModule"), this);
    auto m_api = core->getVadereApi();
    m_api->setConverter(_converter);
    emit(simBoundSignal, this);
    emit(simOffsetSignal, this);
}

////////////////////////////////////////////

void OsgCoordConverterSumo::initialize(int stage) {
  cSimpleModule::initialize(stage);
  if (stage == inet::INITSTAGE_LOCAL) {
      traci::TraCIPosition offset;
      traci::Boundary netBoundary;
      std::string projParameter;
      std::vector<double> netOffset;
      std::vector<double> convBoundary;

      if (hasPar("netFile")){
          // configure based on netFile content
          omnetpp::cXMLElement* netFile = par("netFile").xmlValue();
          auto location = netFile->getFirstChildWithTag("location");
          if(location == nullptr){
              throw cRuntimeError("expected location element");
          }
          netOffset = parse(location->getAttribute("netOffset"), 2);
          convBoundary = parse(location->getAttribute("convBoundary"), 4);
          projParameter = location->getAttribute("projParameter");

      } else {
          // manual configuration
          netOffset = parse(par("netOffset").stdstringValue(), 2);
          convBoundary = parse(par("convBoundary").stdstringValue(), 4);
//          auto origBoundary = parse(par("origBoundary").stdstringValue(), 4);
          projParameter = par("projParameter").stdstringValue();
      }
      netBoundary = traci::Boundary(convBoundary);
      offset = traci::TraCIPosition(netOffset[0], netOffset[1]);

      _converter = std::make_shared<OsgCoordinateConverter>(offset, netBoundary, projParameter);
      _converter->setCellSize(par("cellSize").doubleValue());
      auto areaOfIntrest = dynamic_cast<AreaOfInterest*>(par("areaOfIntrest").objectValue());
      _converter->setAreaOfInterest(areaOfIntrest);

      emit(simBoundSignal, this);
      emit(simOffsetSignal, this);
  }
}

std::vector<double> OsgCoordConverterSumo::parse(const std::string& input, const int count) const{
    omnetpp::cStringTokenizer tokenizer(input.c_str(), ",");
    std::vector<double> ret = tokenizer.asDoubleVector();
    if(ret.size() != count){
        throw cRuntimeError("expected %d tokens but found %zu '%s'", count, ret.size(), input.c_str());
    }
    return ret;
}

////////////////////////////////////////////

} /* namespace crownet */
