#pragma once

#include <omnetpp/clog.h>
#include "inet/common/INETDefs.h"
#include "crownet/common/ModuleAccess.h"
#include "inet/common/geometry/Geometry_m.h"
#include "crownet/common/util/RingBuffer.h"


#define DEBUG_ON_MODULE_ID(_ID) { if (getId() == _ID){DEBUG_TRAP;}}
#define LOG_MOD   simTime() << " " << this->getFullPath() << " : "
#define LOG_MOD2   simTime() << " " << this->getFullPath() << " :   "
#define APP_DBG(_MOD, _TYPE)  \
    cModule* _app = findByPropertyUp(_MOD, #_TYPE); \
    int _node = getContainingNode(_MOD)->getId(); \



namespace std {
std::ostream &operator<<(std::ostream &os, const std::pair<int, int> &pair);

inline std::ostream &operator<<(std::ostream &os,
                                const std::pair<int, int> &pair) {
  return os << "[ " << pair.first << ", " << pair.second << "]";
}
}  // namespace std

namespace crownet {

inline bool operator<(const simtime_t& a, const double b){return a.dbl() < b; }
inline bool operator<(const double a, const simtime_t& b){return a < b.dbl();}

extern omnetpp::cConfigOption *CFGID_VADERE_HOST;
extern omnetpp::cConfigOption *CFGID_SUMO_HOST;
extern omnetpp::cConfigOption *CFGID_FLOW_HOST;

enum DpmmMapType {
    PEDESTRIAN_COUNT=0,
    ENTROPY
};

static const char *dpmmMapTpyeString[] = {"pedestrianCount", "entropyData"};

std::string dpmmMapTypeToString(const DpmmMapType& type);

std::pair<std::string, int> getHostPortConfigOverride(omnetpp::cConfigOption *entry);

uint32_t simtime_to_timestamp_32_ms(simtime_t t = -1.);

simtime_t timestamp_32_ms_to_simtime(uint32_t tstamp, simtime_t base = -1.0);

}
