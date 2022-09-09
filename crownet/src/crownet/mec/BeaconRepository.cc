#include "BeaconRepository.h"

#include <omnetpp.h>

using namespace inet;

void BeaconRepository::add(SimpleMECBeacon beacon)
{
    StoredBeacon sb = {
      .beacon = beacon
    };

    storage[beacon.getNodeId()] = sb;

    EV << "Stored beacon. Repository now contains " << storage.size() << " beacons" << std::endl;
}


std::vector<SimpleMECBeacon> BeaconRepository::getAll()
{
    std::vector<SimpleMECBeacon> beacons;

    for (auto it = storage.begin(); it != storage.end(); ++it)
    {
        beacons.push_back(it->second.beacon);
    }

    return beacons;
}
