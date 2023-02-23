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
#include "crownet/applications/common/info/InfoTags_m.h"

namespace crownet {

Define_Module(ApplicationPacketMeterIn);


ApplicationPacketMeterIn::ApplicationPacketMeterIn() {
    // TODO Auto-generated constructor stub

}

ApplicationPacketMeterIn::~ApplicationPacketMeterIn() {
    delete appLevelInfo;
    for(auto &e: appInfos){
        delete e.second;
    }
    appInfos.clear();
}

void ApplicationPacketMeterIn::initialize(int stage)
{
    GenericPacketMeter::initialize(stage);
    if (stage == INITSTAGE_LOCAL){
        hostId = getContainingNode(this)->getId();

        appendAppInfo = par("appendAppInfo").boolValue();
        appInfoFactor = cObjectFactory::get(par("appInfoClass").stringValue());

        emaSmoothingJitter = par("ema_smoothing_jitter");
        emaSmoothingPacketSize = par("ema_smoothing_packet_size");

        appLevelInfo = dynamic_cast<AppRxInfoPerSource*>(appInfoFactor->createOne());
        take(appLevelInfo);
        if (appLevelInfo == nullptr){
            throw cRuntimeError("Cannot cast appInfoFactor instance to AppInfoBase. Does your AppInfoClass extends AppInfoBase?");
        }
        appLevelInfo->setNodeId(0);
        appLevelInfo->setEma_smoothing_jitter(emaSmoothingJitter);
        appLevelInfo->setEma_smoothing_packet_size(emaSmoothingPacketSize);

        WATCH_PTR(appLevelInfo);
        WATCH_PTRMAP(appInfos);
    }
}

void ApplicationPacketMeterIn::meterPacket(Packet *packet)
{
    // todo how to handle self messages? aka hostId == sourceId
    GenericPacketMeter::meterPacket(packet);
    auto data = packet->peekData();
    int sourceId = data->getAllTags<HostIdTag>().front().getTag()->getHostId();
    auto now = simTime();

    // process source level statistics
    AppRxInfoPerSource* info = getOrCreate(sourceId);
    info->processInbound(packet, hostId, now);

    // process application level statistics
    appLevelInfo->processInbound(packet, hostId, now);


    if (appendAppInfo){
        // append copy of current statistic object for application internal processing
        auto tag = packet->addTagIfAbsent<AppRxInfoPerSourceTag>();
        auto info_copy = info->dup();
        tag->setPacketInfo(info_copy);
    }
    packet->addTagIfAbsent<RxPerSrcJitterTag>()->setJitter(info->getJitter());
    packet->addTagIfAbsent<RxPerSrcAvgSizeTag>()->setSize(info->getAvg_packet_size());
    packet->addTagIfAbsent<RxPerSrcPktCountTag>()->setCount(info->getPacketsReceivedCount());
    packet->addTagIfAbsent<RxPerSrcPktLossCountTag>()->setCount(info->getPacketsLossCount());

    packet->addTagIfAbsent<RxPerAppAvgSizeTag>()->setSize(appLevelInfo->getAvg_packet_size());
    packet->addTagIfAbsent<RxPerAppPktCountTag>()->setCount(appLevelInfo->getPacketsReceivedCount());


    packet->addTagIfAbsent<RxSourceCountTag>()->setRxSourceCount(getNumberOfSenders());
}

const AppRxInfo* ApplicationPacketMeterIn::getAppRxInfo(const int id) const {
    if (id < 0){
        return appLevelInfo;
    } else {
      const auto iter = appInfos.find(id);
      if (iter == appInfos.end()){
          throw cRuntimeError("No AppInfo object for nodeId %d", id);
      } else {
          return iter->second;
      }
    }
}

const int ApplicationPacketMeterIn::getNumberOfSenders() const {
    return appInfos.size();
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

