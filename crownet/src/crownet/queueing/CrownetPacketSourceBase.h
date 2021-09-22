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

#ifndef CROWNET_QUEUEING_CROWNETPACKETSOURCEBASE_H_
#define CROWNET_QUEUEING_CROWNETPACKETSOURCEBASE_H_

#include "inet/queueing/base/PacketSourceBase.h"

using namespace inet;

namespace crownet{
namespace queueing{

class CrownetPacketSourceBase : public inet::queueing::PacketSourceBase {

protected:
    const char *packetName = nullptr;

protected:
    virtual void initialize(int stage) override;

    virtual const char *createPacketName(const Ptr<const Chunk>& data) const override;
    virtual void applyContentTags(Ptr<Chunk> content);
    virtual void applyPacketTags( Packet *);
};


}
}


#endif /* CROWNET_QUEUEING_CROWNETPACKETSOURCEBASE_H_ */
