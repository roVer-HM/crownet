#include "BeaconAggregationStrategy.h"
#include <cmath>

Packet* V2P_P2VFilter::createBeaconPacket(std::vector<SimpleMECBeacon> beacons)
{
    std::vector <SimpleMECBeacon> filtered;

    for (auto& beacon : beacons)
    {
        if (
                (ueType == 1 && beacon.getNodeType() == 2) ||
                (ueType == 2 && beacon.getNodeType() == 1) ||
                ueId == beacon.getNodeId()
        ) {
            filtered.push_back(beacon);
        }
    }

    EV << "Contains n beacons: " << filtered.size() << std::endl;

    if (filtered.size() == 0) return nullptr;

    return aggregateBeacons(filtered);


}

bool V2P_P2VFilter::handleUEBeacon(SimpleMECBeacon beacon)
{
    selfPosition.xPos = beacon.getXPos();
    selfPosition.yPos = beacon.getYPos();
    ueType = beacon.getNodeType();
    ueId = beacon.getNodeId();

    return true;
}
