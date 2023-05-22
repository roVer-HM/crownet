#include "inet/common/INETMath.h"
#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/FlowTag.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/ResultRecorders.h"
#include "inet/common/Simsignals_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "artery/traci/PersonMobility.h"
#include "CustomFilter.h"

namespace inet {
namespace utils {
namespace filters {

Register_ResultFilter("personId", PersonIdFilter);

void PersonIdFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t,
        cObject *object, cObject *details) {

    if (auto wrapper = dynamic_cast<artery::PersonMobility*>(object)) {
        fire(this, t, 1.00, details);
    }

}

}
}
}
