/*
 * OutboundDensitzMap.cc
 *
 *  Created on: Jul 14, 2021
 *      Author: matthias
 */

#include "OutboundDensityMap.h"

OutboundDensityMap::OutboundDensityMap()
{

}

OutboundDensityMap::~OutboundDensityMap()
{

}

void OutboundDensityMap::addSingle(SingleLocationData const * const data)
{
    int deviceId = data->deviceid();

    auto it = nodeDensityMap.find(deviceId);

    // Return if there already exists a more recent entry for this device
    if (it != nodeDensityMap.end() && (*it).second.timestamp() > data->timestamp()) {
        return;
    }

    nodeDensityMap[deviceId] = *data;
}

void OutboundDensityMap::reset()
{
    nodeDensityMap.clear();
}

void OutboundDensityMap::writeToDensityMap(DensityMap* map)
{
    for (auto it = nodeDensityMap.begin(); it != nodeDensityMap.end(); it++) {
        auto data = map->add_data();
        data->CopyFrom((*it).second);
    }
}
