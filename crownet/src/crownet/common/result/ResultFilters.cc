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

#include "crownet/common/result/ResultFilters.h"
#include "inet/common/ResultFilters.h"

#include "inet/common/packet/Packet.h"
#include "crownet/applications/detour/DetourAppPacket_m.h"
#include "crownet/applications/common/AppCommon_m.h"
#include "crownet/common/converter/OsgCoordConverter.h"

using namespace inet;

namespace crownet {

Register_ResultFilter("simTimeIntervalFilter", SimTimeIntervalFilter);
void SimTimeIntervalFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t,
                                      cObject *object, cObject *details) {
    if (filterInterval < 0 || t >= nextLogTime){
        nextLogTime = t + filterInterval;
        fire(this, t, object, details);
    }
}

void SimTimeIntervalFilter::init(cComponent *component, cProperty *attrsProperty){
    cPointerResultFilter::init(component, attrsProperty);
    if(attrsProperty->containsKey("interval_par")){
        auto _val = attrsProperty->getValue("interval_par");
        if(component->hasPar(_val)){
            this->filterInterval = component->par(_val);
        } else {
            throw cRuntimeError("simTimeIntervalFilter expected '%s' parameter on module '%s'", _val, component->getFullPath().c_str());
        }
    } else {
        throw cRuntimeError("simTimeIntervalFilter expected a statistic attribute 'interval_par'");
    }

}

Register_ResultFilter("incidentAge", IncidentAgeFilter);
void IncidentAgeFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t,
                                      cObject *object, cObject *details) {
  if (auto packet = dynamic_cast<Packet *>(object)) {
    auto data = packet->peekData<DetourAppPacket>();
    simtime_t delta = simTime() - data->getFirstHopTime();
    ASSERT(delta >= 0);  // delta == 0 if packet comes with loopback
    fire(this, t, delta, details);
  }
}

Register_ResultFilter("lastHopAge", LastHopAgeFilter);
void LastHopAgeFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t,
                                     cObject *object, cObject *details) {
  if (auto packet = dynamic_cast<Packet *>(object)) {
    auto data = packet->peekData<DetourAppPacket>();
    simtime_t delta = simTime() - data->getLastHopTime();
    ASSERT(delta >= 0);  // delta == 0 if packet comes with loopback
    fire(this, t, delta, details);
  }
}

Register_ResultFilter("rcvdHostId", RcvdHostIdFilter);
void RcvdHostIdFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t,
                                     cObject *object, cObject *details) {
  if (auto packet = dynamic_cast<Packet *>(object)) {
      for(auto& region : packet->peekData()->getAllTags<HostIdTag>()){
          fire(this, t, (double)(region.getTag()->getHostId()), details);
      }
  }
}

Register_ResultFilter("hostId", HostIdFilter);
void HostIdFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t,
                                     cObject *object, cObject *details) {
  if (auto notifier = dynamic_cast<cPostModuleBuildNotification*>(object)){
      if(notifier->module->getProperties()->getAsBool("networkNode")){
          fire(this, t, (double)notifier->module->getId(), details);
      }
  }
}

Register_ResultFilter("rcvdSequenceId", RcvdSequenceIdFilter);
void RcvdSequenceIdFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t,
                                     cObject *object, cObject *details) {
  if (auto packet = dynamic_cast<Packet *>(object)) {
      for(auto& region : packet->peekData()->getAllTags<SequenceIdTag>()){
          fire(this, t, (double)(region.getTag()->getSequenceNumber()), details);
      }
  }
}

Register_ResultFilter("simBound", SimBoundFilter);
void SimBoundFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details){
    OsgCoordConverterLocal *module = dynamic_cast<OsgCoordConverterLocal *>(object);
     if (module) {
         Coord coord = module->getConverter()->getBoundary();
         inet::utils::filters::VoidPtrWrapper wrapper(&coord);
         fire(this, t, &wrapper, details);
     }
}


Register_ResultFilter("simOffset", SimOffsetFilter);
void SimOffsetFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details){
    OsgCoordConverterLocal *module = dynamic_cast<OsgCoordConverterLocal *>(object);
     if (module) {
         Coord coord = module->getConverter()->getOffset();
         inet::utils::filters::VoidPtrWrapper wrapper(&coord);
         fire(this, t, &wrapper, details);
     }
}


}  // namespace crownet
