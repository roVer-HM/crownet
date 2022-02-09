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

#include "CrownetPacketSourceBase.h"

#include "inet/common/INETDefs_m.h"
#include "inet/common/packet/chunk/Chunk_m.h"
#include "inet/common/DirectionTag_m.h"
#include "inet/common/IdentityTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "crownet/applications/common/AppCommon_m.h"

using namespace inet;

namespace crownet{
namespace queueing{

void CrownetPacketSourceBase::initialize(int stage)
{
    inet::queueing::PacketSourceBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        packetName = par("packetName");
        attachHostIdTag = par("attachHostIdTag").boolValue();
        attachSequenceIdTag = par("attachSequenceIdTag").boolValue();
        hostId = getContainingNode(this)->getId();
        WATCH(hostId);
    }
}

const char *CrownetPacketSourceBase::createPacketName(const Ptr<const Chunk>& data) const
{
    return StringFormat::formatString(packetNameFormat, [&] (char directive) {
        static std::string result;
        switch (directive) {
            case 'N':
                result = packetName;
                break;
            case 'n':
                result = getFullName();
                break;
            case 'p':
                result = getFullPath();
                break;
            case 'c':
                result = std::to_string(numProcessedPackets);
                break;
            case 'l':
                result = data->getChunkLength().str();
                break;
            case 'd':
                if (auto byteCountChunk = dynamicPtrCast<const ByteCountChunk>(data))
                    result = std::to_string(byteCountChunk->getData());
                else if (auto bitCountChunk = dynamicPtrCast<const BitCountChunk>(data))
                    result = std::to_string(bitCountChunk->getData());
                break;
            case 't':
                result = simTime().str();
                break;
            case 'e':
                result = std::to_string(getSimulation()->getEventNumber());
                break;
            case 'i':
                result = std::to_string(hostId);
                break;
            default:
                throw cRuntimeError("Unknown directive: %c", directive);
        }
        return result.c_str();
    });
}

void CrownetPacketSourceBase::applyContentTags(Ptr<Chunk> content){
    if (attachCreationTimeTag)
        content->addTag<CreationTimeTag>()->setCreationTime(simTime());
    if (attachIdentityTag) {
        auto identityStart = IdentityTag::getNextIdentityStart(content->getChunkLength());
        content->addTag<IdentityTag>()->setIdentityStart(identityStart);
    }
    if (attachHostIdTag){
        content->addTag<HostIdTag>()->setHostId(hostId);
    }
    if (attachSequenceIdTag){
        content->addTag<SequenceIdTag>()->setSequenceNumber(numProcessedPackets);
    }
}
void CrownetPacketSourceBase::applyPacketTags(Packet *packet){
    if (attachDirectionTag)
        packet->addTagIfAbsent<DirectionTag>()->setDirection(DIRECTION_OUTBOUND);

}



}
}
