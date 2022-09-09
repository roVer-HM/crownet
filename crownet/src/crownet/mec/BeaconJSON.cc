#include "BeaconJSON.h"

nlohmann::json beacon_json::serializeBeacon(const SimpleMECBeacon& beaconIn)
{
    nlohmann::json beaconJSON = {
            {"nid", beaconIn.getNodeId()},
            {"type", beaconIn.getNodeType()},
            {"xpos", beaconIn.getXPos()},
            {"ypos", beaconIn.getYPos()}
    };

    return beaconJSON;
}

nlohmann::json beacon_json::serializeBeacons(std::vector<SimpleMECBeacon> beaconsIn)
{
    nlohmann::json array = nlohmann::json::array();

    for (auto it = beaconsIn.begin(); it != beaconsIn.end(); ++it)
    {
        array.push_back(beacon_json::serializeBeacon(*it));
    }

    return array;
}


SimpleMECBeacon beacon_json::parseBeacon(nlohmann::ordered_json jsonIn)
{
    SimpleMECBeacon beacon;
    beacon.setXPos(jsonIn["xpos"].get<double>());
    beacon.setYPos(jsonIn["ypos"].get<double>());
    beacon.setNodeId(jsonIn["nid"].get<int>());
    beacon.setNodeType(jsonIn["type"].get<int>());

    return beacon;
}


SimpleMECBeacon beacon_json::parseBeaconString(const std::string& stringIn)
{
    nlohmann::ordered_json json = nlohmann::json::parse(stringIn);
    return parseBeacon(json);
}


std::vector<SimpleMECBeacon> beacon_json::parseBeacons(nlohmann::ordered_json jsonIn)
{
    std::vector<SimpleMECBeacon> beacons;

    for (auto it = jsonIn.begin(); it != jsonIn.end(); ++it)
    {
        beacons.push_back(parseBeacon(*it));
    }

    return beacons;
}

std::vector<SimpleMECBeacon> beacon_json::parseBeaconsString(const std::string& stringIn)
{
    nlohmann::ordered_json json = nlohmann::json::parse(stringIn);
    return parseBeacons(json);
}
