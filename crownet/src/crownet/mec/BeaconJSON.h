#ifndef BEACONJSON_H
#define BEACONJSON_H

#include "packets/SimpleMECBeacon_m.h"
#include <nodes/mec/utils/httpUtils/httpUtils.h>

#include <vector>

namespace beacon_json {
    nlohmann::json serializeBeacon(const SimpleMECBeacon& beaconIn);
    nlohmann::json serializeBeacons(std::vector<SimpleMECBeacon> beaconIn);

    SimpleMECBeacon parseBeacon(nlohmann::ordered_json jsonIn);
    std::vector<SimpleMECBeacon> parseBeacons(nlohmann::ordered_json jsonIn);
    SimpleMECBeacon parseBeaconString(const std::string& stringIn);
    std::vector<SimpleMECBeacon> parseBeaconsString(const std::string& stringIn);
};

#endif
