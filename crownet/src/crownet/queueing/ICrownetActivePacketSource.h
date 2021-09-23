
#pragma once

#include "inet/queueing/contract/IActivePacketSource.h"


namespace crownet{
namespace queueing{


class ICrownetActivePacketSource : public virtual  inet::queueing::IActivePacketSource {

public:

    // produce a single packet
    virtual void producePacket() = 0;
    virtual void producePackets(int number) = 0;
    // produce number of packet(s) without exceeding maxData
    virtual void producePackets(inet::B maxData) = 0;

};

}
}
