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

#include "CrownetActivePacketSourceBase.h"


#include "inet/common/ModuleAccess.h"

namespace crownet {
namespace queueing {

void CrownetActivePacketSourceBase::initialize(int stage)
{
    CrownetPacketSourceBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        outputGate = gate("out");
        consumer = findConnectedModule<inet::queueing::IPassivePacketSink>(outputGate);
    }
    else if (stage == INITSTAGE_QUEUEING)
        checkPacketOperationSupport(outputGate);
}

void CrownetActivePacketSourceBase::producePackets(int number) {
    Enter_Method_Silent("producePackets");
    for (int i=0; i< number; i++){
        producePacket();
    }
}

void CrownetActivePacketSourceBase::producePacket()
{
    Enter_Method("producePacket");
    auto packet = createPacket();
    EV_INFO << "Producing packet" << EV_FIELD(packet) << EV_ENDL;
    handlePacketProcessed(packet);
    pushOrSendPacket(packet, outputGate, consumer);
    updateDisplayString();
}

void CrownetActivePacketSourceBase::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    producePacket();

}

void CrownetActivePacketSourceBase::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
}


} // namespace queueing
} // namespace inet
