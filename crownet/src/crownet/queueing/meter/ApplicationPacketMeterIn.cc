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

#include "ApplicationPacketMeterIn.h"
#include "inet/common/TimeTag_m.h"
#include "crownet/applications/common/AppCommon_m.h"
#include "crownet/common/util/FilePrinter.h"
#include "crownet/applications/common/info/AppRxInfoPerSource.h"

namespace crownet {

Define_Module(ApplicationPacketMeterIn);


ApplicationPacketMeterIn::ApplicationPacketMeterIn() {
    // TODO Auto-generated constructor stub

}

ApplicationPacketMeterIn::~ApplicationPacketMeterIn() {
    // TODO Auto-generated destructor stub
}

void ApplicationPacketMeterIn::initialize(int stage)
{
    GenericPacketMeter::initialize(stage);
    if (stage == INITSTAGE_LOCAL){
        hostId = getContainingNode(this)->getId();

        appendAppInfoTag = par("appendAppInfoTag").boolValue();
        appInfoFactor = cObjectFactory::get(par("appInfoClass").stringValue());


        appLevelInfo = dynamic_cast<AppRxInfoPerSource*>(appInfoFactor->createOne());
        if (appLevelInfo == nullptr){
            throw cRuntimeError("Cannot cast appInfoFactor instance to AppInfoBase. Does your AppInfoClass extends AppInfoBase?");
        }
        appLevelInfo->setNodeId(-1);
    }
}

void ApplicationPacketMeterIn::meterPacket(Packet *packet)
{
    // todo how to handle self messages? aka hostId == sourceId
    GenericPacketMeter::meterPacket(packet);
    auto data = packet->peekData();
    int sourceId = data->getTag<HostIdTag>()->getHostId();

    // process application level statistics
    appLevelInfo->processInbound(packet, hostId, simTime());

    // process source level statistics
    AppRxInfoPerSource* info = getOrCreate(sourceId);
    info->processInbound(packet, hostId, simTime());
    if (appendAppInfoTag){
        packet->addTagIfAbsent<AppInfoTag>()->setAppInfo(info);
    }
}

AppRxInfoPerSource* ApplicationPacketMeterIn::getOrCreate(int sourceId){

    if(appInfos.find(sourceId) == appInfos.end()){
        // no data from this host id. create new
        auto newInfo = dynamic_cast<AppRxInfoPerSource*>(appInfoFactor->createOne());
        if (newInfo == nullptr){
            throw cRuntimeError("Cannot cast appInfoFactor instance to AppInfoBase. Does your AppInfoClass extends AppInfoBase?");
        }
        newInfo->setNodeId(sourceId);
        newInfo->setEma_smoothing_jitter(emaSmoothingJitter);
        newInfo->setEma_smoothing_packet_size(emaSmoothingPacketSize);
        take(newInfo);
        appInfos[sourceId] = newInfo;
    }
    return appInfos[sourceId];
}


}//namespace

