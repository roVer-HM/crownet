#include "BeaconAggregationStrategy.h"

#include "inet/common/TimeTag_m.h"

#include <cmath>

Packet* BeaconAggregationStrategy::aggregateBeacons(std::vector<SimpleMECBeacon> beacons)
{
    inet::Packet* pkt = new inet::Packet("ResponseBeaconMessage");
    auto rbm = inet::makeShared<ResponseBeaconMessage>();

    rbm->setBeaconsArraySize(beacons.size());

    int i = 0;

    for (const auto& b : beacons)
    {
        ReducedMECBeacon rb;
        rb.setXPos(b.getXPos());
        rb.setYPos(b.getYPos());
        rb.setId(b.getNodeId());
        rb.setNodeType(b.getNodeType());
        rb.setAdditionalDataArraySize(100);

        for (int i = 0; i < 100; i++) rb.setAdditionalData(i, std::rand());

        rbm->setBeacons(i++, rb);
    }

    int chunkSize = beacons.size() > 0 ? beacons.size() * 400 : 20;

    rbm->setChunkLength(inet::B(chunkSize));
    rbm->addTagIfAbsent<inet::CreationTimeTag>()->setCreationTime(simTime());

    pkt->insertAtBack(rbm);

    return pkt;
}

double BeaconAggregationStrategy::dist(double x1, double y1, double x2, double y2)
{
    using namespace std;
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

double BeaconAggregationStrategy::dist(SimpleMECBeacon beacon, double x2, double y2)
{
    return dist(beacon.getXPos(), beacon.getYPos(), x2, y2);
}
