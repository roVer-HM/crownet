//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include <glob.h>
#include <omnetpp.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "crownet/artery/traci/VadereUtils.h"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>


using namespace omnetpp;

namespace vadere {


std::vector<VadereCache> getCachePaths(const std::string vadereCachePath,
                                       const std::string hash) {
  std::vector<VadereCache> cachePaths;
  std::string cacheLocation = vadereCachePath + "/" + hash + "*";

  glob_t glob_res;

  glob(cacheLocation.c_str(), GLOB_TILDE, nullptr, &glob_res);
  for (unsigned int i = 0; i < glob_res.gl_pathc; ++i) {
    VadereCache c;
    std::string file = std::string(glob_res.gl_pathv[i]);
    size_t p1 = file.rfind('/');
    size_t p2 = file.find('_', p1);
    size_t p3 = file.find('.', p1);
    EV_INFO << file << std::endl;
    std::string cacheIdentifier = file.substr(p2, p3 - p2);
    EV_INFO << p1 << ',' << p2 << ',' << p3 << std::endl;
    c.first = cacheIdentifier;
    c.second = file;
    cachePaths.push_back(c);
  }
  globfree(&glob_res);
  return cachePaths;
}

VadereScenario getScenarioContent(const std::string vadereScenarioPath) {
  VadereScenario ret;
  ret.first = vadereScenarioPath;

  std::ifstream file(ret.first.c_str());
  if (file) {
    ret.second = std::string((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
  } else {
    throw cRuntimeError("Cannot read scenario file at: \"%s\"",
                        ret.first.c_str());
  }
  return ret;
}

CoordianteSystemSettings getCoordianteSystemSettings(
        const std::string vadereScenarioPath){
    std::stringstream ss;
    ss << getScenarioContent(vadereScenarioPath).second;

    boost::property_tree::ptree pt;
    boost::property_tree::read_json(ss, pt);
    CoordianteSystemSettings ret;
    try {
        ret.bound = {
            pt.get<double>("scenario.topography.attributes.bounds.x"),
            pt.get<double>("scenario.topography.attributes.bounds.y"),
            pt.get<double>("scenario.topography.attributes.bounds.width"),
            pt.get<double>("scenario.topography.attributes.bounds.height"),
        };
        ret.epsg = pt.get<std::string>("scenario.topography.attributes.referenceCoordinateSystem.epsgCode");
        ret.offsetX = pt.get<double>("scenario.topography.attributes.referenceCoordinateSystem.translation.x");
        ret.offsetY = pt.get<double>("scenario.topography.attributes.referenceCoordinateSystem.translation.y");
    } catch (std::exception const& e) {
        throw cRuntimeError("Error parsing Vadere scenario file: %s", e.what());
    }
    return ret;
}



}  // namespace vadere
