#include <vector>
#include <omnetpp.h>

#include "../packets/SimpleMECBeacon_m.h"
#include "inet/common/packet/Packet.h"

#ifndef BEACONAGGREGATIONSTRATEGY_H
#define BEACONAGGREGATIONSTRATEGY_H

using namespace inet;

class BeaconAggregationStrategy
{
public:
    /**
     * Create packet from a list of beacons
     */
    virtual Packet* createBeaconPacket(std::vector <SimpleMECBeacon>) = 0;

    /**
     * Called whenever a new beacon from the connected UE arrives at the MECBeaconApp
     * \return true if this beacon should be saved, otherwise false
     */
    virtual bool handleUEBeacon(SimpleMECBeacon) { return true; };

    /**
     * Returns the interval at which beacons should be disseminated to the connected UE.
     * \param defaultInterval the default interval (specified as "period" in MECBeaconApp.ned)
     * \return the interval that should be used
     */
    virtual simtime_t getInterval(simtime_t defaultInterval) { return defaultInterval; }

protected:
    /**
     * Helper function to aggregate multiple beacons to a single packet
     */
    Packet* aggregateBeacons(std::vector <SimpleMECBeacon>);
    /**
     * Euclidean distance
     */
    double dist(double, double, double, double);
    double dist(SimpleMECBeacon, double, double);
};


class DefaultBeaconAggregationStrategy: public BeaconAggregationStrategy
{
public:
    virtual Packet* createBeaconPacket(std::vector <SimpleMECBeacon>) override;

    virtual bool handleUEBeacon(SimpleMECBeacon) override { return true; };
};


class DangerZoneBeaconFilter: public BeaconAggregationStrategy
{
public:
    virtual Packet* createBeaconPacket(std::vector <SimpleMECBeacon>) override;
    virtual bool handleUEBeacon(SimpleMECBeacon) override;

    void addDangerZone(float x, float y, float radius);

private:
    struct DangerZone {
      float x;
      float y;
      float radius;
    };

    std::vector<DangerZone> dangerZones;

    int ueType = 0;
};


class PriorityBasedAggregator: public BeaconAggregationStrategy
{
public:
    PriorityBasedAggregator() {};
    PriorityBasedAggregator(int, int);

    virtual Packet* createBeaconPacket(std::vector <SimpleMECBeacon>) override;
    virtual bool handleUEBeacon(SimpleMECBeacon) override;

    int nPriorityGroups = 3;
    int priorityGroupThresholdDistance = 10;

private:
    struct {
        double xPos;
        double yPos;
    } selfPosition;

    unsigned int beaconCounter = 1;

    cOutVector receivedNodeID;
    cOutVector sentNodeID;

    int ueType = 0;
    int ueId = 0;
};

class V2P_P2VFilter: public BeaconAggregationStrategy
{
public:
    virtual Packet* createBeaconPacket(std::vector <SimpleMECBeacon>) override;
    virtual bool handleUEBeacon(SimpleMECBeacon) override;

private:
    struct {
        double xPos;
        double yPos;
    } selfPosition;

    int ueType = 0;
    int ueId = 0;
};


#endif
