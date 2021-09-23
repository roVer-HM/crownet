#pragma once

#include "inet/common/INETDefs.h"
#include "inet/common/geometry/Geometry_m.h"
#include "crownet/common/util/RingBuffer.h"


#define DEBUG_ON_MODULE_ID(_ID) { if (getId() == _ID){DEBUG_TRAP;}}

namespace std {
std::ostream &operator<<(std::ostream &os, const std::pair<int, int> &pair);

inline std::ostream &operator<<(std::ostream &os,
                                const std::pair<int, int> &pair) {
  return os << "[ " << pair.first << ", " << pair.second << "]";
}
}  // namespace std

namespace crownet {

extern omnetpp::cConfigOption *CFGID_VADERE_HOST;
extern omnetpp::cConfigOption *CFGID_SUMO_HOST;
extern omnetpp::cConfigOption *CFGID_FLOW_HOST;

std::pair<std::string, int> getHostPortConfigOverride(omnetpp::cConfigOption *entry);

}
