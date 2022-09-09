#include <vector>
#include <omnetpp.h>

#include "../packets/SimpleMECBeacon_m.h"
#include <inet/common/packet/Packet.h>

#ifndef BEACONAGGREGATIONSTRATEGY_H
#define BEACONAGGREGATIONSTRATEGY_H

using namespace inet;

class BeaconAggregationStrategy
{
public:
    virtual Packet* createBeaconPacket(std::vector <SimpleMECBeacon>) = 0;
};

class DefaultBeaconAggregationStrategy: public BeaconAggregationStrategy
{
public:
    virtual Packet* createBeaconPacket(std::vector <SimpleMECBeacon>);
};

#endif
