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

#ifndef CROWNET_QUEUEING_METER_APPLICATIONPACKETMETERIN_H_
#define CROWNET_QUEUEING_METER_APPLICATIONPACKETMETERIN_H_

#include "crownet/queueing/meter/GenericPacketMeter.h"
#include "crownet/applications/common/info/AppRxInfoPerSource.h"

namespace crownet {

using SourceAppInfoMap = std::map<int, AppRxInfoPerSource*>;

class ApplicationPacketMeterIn : public GenericPacketMeter {
public:
    ApplicationPacketMeterIn();
    virtual ~ApplicationPacketMeterIn();
protected:
    virtual void initialize(int stage) override;
    virtual void meterPacket(inet::Packet *packet) override;
    virtual AppRxInfoPerSource* getOrCreate(int sourceId);

protected:
    int hostId;
    AppRxInfoPerSource* appLevelInfo = nullptr;

    bool appendAppInfoTag = false;
    cObjectFactory* appInfoFactor = nullptr;
    double emaSmoothingJitter;
    double emaSmoothingPacketSize;
    SourceAppInfoMap appInfos;
};

}// namespace
#endif /* CROWNET_QUEUEING_METER_APPLICATIONPACKETMETERIN_H_ */
