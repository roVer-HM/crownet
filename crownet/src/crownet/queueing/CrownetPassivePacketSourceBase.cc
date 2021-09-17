//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "CrownetPassivePacketSourceBase.h"

#include "inet/queueing/source/PassivePacketSource.h"

#include "inet/common/INETDefs_m.h"
#include "inet/common/packet/chunk/Chunk_m.h"
#include "inet/common/ModuleAccess.h"

using namespace inet;

namespace crownet{
namespace queueing{


void CrownetPassivePacketSourceBase::initialize(int stage)
{
    CrownetPacketSourceBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        outputGate = gate("out");
        collector = findConnectedModule<inet::queueing::IActivePacketSink>(outputGate);
    }
    else if (stage == INITSTAGE_QUEUEING)
        checkPacketOperationSupport(outputGate);
}

Packet *CrownetPassivePacketSourceBase::canPullPacket(cGate *gate) const {
    Enter_Method("canPullPacket");
    if (nextPacket == nullptr){
        nextPacket = const_cast<CrownetPassivePacketSourceBase *>(this)->createPacket();
    }

    return nextPacket;
}

Packet *CrownetPassivePacketSourceBase::pullPacket(cGate *gate)
{
    Enter_Method("pullPacket");

    auto packet = providePacket(gate);
    // statistics
    handlePacketProcessed(packet);
    animateSendPacket(packet, outputGate);
    emit(packetPulledSignal, packet);
    updateDisplayString();
    return packet;
}

Packet *CrownetPassivePacketSourceBase::providePacket(cGate *gate)
{
    Packet *packet;
    if (nextPacket == nullptr)
        packet = createPacket();
    else {
        packet = nextPacket;
        nextPacket = nullptr;
    }
    EV_INFO << "Providing packet" << EV_FIELD(packet) << EV_ENDL;
    updateDisplayString();
    return packet;
}


}
}
