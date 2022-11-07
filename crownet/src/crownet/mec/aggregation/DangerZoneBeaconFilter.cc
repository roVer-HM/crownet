#include "BeaconAggregationStrategy.h"

#include <cmath>

void DangerZoneBeaconFilter::addDangerZone(float x, float y, float radius)
{
    dangerZones.push_back((DangerZone) {x, y, radius});
}


Packet* DangerZoneBeaconFilter::createBeaconPacket(std::vector <SimpleMECBeacon> beacons)
{
    using namespace std;

    // Only send beacons to vehicle nodes
    if (ueType != 2) return nullptr;

    std::vector <SimpleMECBeacon> filtered;

    for (auto& beacon : beacons)
    {
        bool inDangerZone = false;

        for (auto& dz : dangerZones)
        {
            if (dist(beacon, dz.x, dz.y) < dz.radius)
            {
                inDangerZone = true;

                // Already in danger zone. No need to check the others
                break;
            }
        }
        // In danger zone -> keep
        if (inDangerZone) filtered.push_back(beacon);
    }

    return aggregateBeacons(filtered);
}

bool DangerZoneBeaconFilter::handleUEBeacon(SimpleMECBeacon beacon)
{
    ueType = beacon.getNodeType();

    return true;
}
