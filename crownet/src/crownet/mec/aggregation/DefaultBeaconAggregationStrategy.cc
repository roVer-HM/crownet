#include "BeaconAggregationStrategy.h"

#include "inet/common/TimeTag_m.h"

Packet* DefaultBeaconAggregationStrategy::createBeaconPacket(std::vector <SimpleMECBeacon> beacons)
{
    return aggregateBeacons(beacons);
}
