#include "BeaconAggregationStrategy.h"
#include <cmath>

PriorityBasedAggregator::PriorityBasedAggregator(int pg, int td)
{
    nPriorityGroups = pg;
    priorityGroupThresholdDistance = td;

    receivedNodeID.setName("receivedMEC");
    sentNodeID.setName("sentMEC");
}

Packet* PriorityBasedAggregator::createBeaconPacket(std::vector<SimpleMECBeacon> beacons)
{
    std::vector <SimpleMECBeacon> filtered;

    EV << "Start priority based beacon dissemination." << std::endl;

    for (int p = 1; p <= nPriorityGroups; ++p)
    {
        EV << "Prio: " << p << std::endl;

        for (auto& beacon : beacons)
        {
            if (ueId != beacon.getNodeId() &&
                (ueType == 0 || (ueType == 1 && beacon.getNodeType() == 1) || (ueType == 2 && beacon.getNodeType() == 2))
            ) continue;

            double distance = dist(beacon, selfPosition.xPos, selfPosition.yPos);

            double minDist = (p - 1) * priorityGroupThresholdDistance;
            double maxDist = (p) * priorityGroupThresholdDistance;

            if (distance >= minDist && distance <= maxDist)
            {
                EV << "Node " << beacon.getNodeId() << " in group " << p << std::endl;

                if (beaconCounter % p == 0)
                {
                    filtered.push_back(beacon);
                    sentNodeID.record(beacon.getNodeId());
                }
            }
        }

    }

    EV << "Contains n beacons: " << filtered.size() << std::endl;

    beaconCounter++;

    if (filtered.size() == 0) return nullptr;

    return aggregateBeacons(filtered);
}

bool PriorityBasedAggregator::handleUEBeacon(SimpleMECBeacon beacon)
{
    selfPosition.xPos = beacon.getXPos();
    selfPosition.yPos = beacon.getYPos();
    ueType = beacon.getNodeType();
    ueId = beacon.getNodeId();

    receivedNodeID.record(beacon.getNodeId());

    return true;
}
