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

#include "ResultRecorders.h"
#include "inet/common/packet/Packet.h"
#include "rover/applications/udpapp/DetourAppPacket_m.h"

using namespace inet;
using namespace omnetpp::common;  // Expression

namespace rover {
Register_ResultRecorder("incident", IncidentRecorder);

std::vector<opp_string> IncidentRecorder::resultNames{
    "firstHopId", "firstHopTime", "firstHopX", "firstHopY",
    "lastHopId",  "lastHopTime",  "lastHopX",  "lastHopY"};

IncidentRecorder::~IncidentRecorder() {}

void IncidentRecorder::subscribedTo(cResultFilter *prev) {
  cResultRecorder::subscribedTo(prev);
}

void IncidentRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t,
                                     bool b, cObject *details) {
  throw cRuntimeError("%s: Expected DetourAppPacket*", getClassName());
}

void IncidentRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t,
                                     intval_t l, cObject *details) {
  throw cRuntimeError("%s: Expected DetourAppPacket*", getClassName());
}

void IncidentRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t,
                                     uintval_t l, cObject *details) {
  throw cRuntimeError("%s: Expected DetourAppPacket*", getClassName());
}

void IncidentRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t,
                                     double d, cObject *details) {
  throw cRuntimeError("%s: Expected DetourAppPacket*", getClassName());
}

void IncidentRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t,
                                     const SimTime &v, cObject *details) {
  throw cRuntimeError("%s: Expected DetourAppPacket*", getClassName());
}

void IncidentRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t,
                                     const char *s, cObject *details) {
  throw cRuntimeError("%s: Expected DetourAppPacket*", getClassName());
}

void IncidentRecorder::registerVectors(opp_string incident) {
  IncidentVectors vectors;
  opp_string_map attr = getStatisticAttributes();
  for (auto &v : resultNames) {
    opp_string vecName = incident + ":" + v;
    vectors[v] = getEnvir()->registerOutputVector(
        getComponent()->getFullPath().c_str(), vecName.c_str());
    ASSERT(vectors[v] != nullptr);
    for (auto &a : attr)
      getEnvir()->setVectorAttribute(vectors[v], a.first.c_str(),
                                     a.second.c_str());
  }
  vectorMap.insert(std::pair<opp_string, IncidentVectors>(incident, vectors));
}

void IncidentRecorder::unregisterVectors() {
  for (auto &incident : vectorMap) {
    for (auto &vec : incident.second) {
      getEnvir()->deregisterOutputVector(vec.second);
    }
  }
}

void IncidentRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t,
                                     cObject *obj, cObject *details) {
  if (auto pkt = dynamic_cast<Packet *>(obj)) {
    auto data = pkt->peekData<DetourAppPacket>();
    std::map<opp_string, IncidentVectors>::iterator it =
        vectorMap.find(data->getIncidentReason());

    if (it == vectorMap.end()) {
      registerVectors(data->getIncidentReason());
      it = vectorMap.find(data->getIncidentReason());
    }
    ASSERT(it != vectorMap.end());
    simtime_t t = simTime();
    IncidentVectors vectors = it->second;

    getEnvir()->recordInOutputVector(vectors["firstHopId"], t,
                                     data->getFirstHopId());
    getEnvir()->recordInOutputVector(vectors["firstHopTime"], t,
                                     data->getFirstHopTime().dbl());
    getEnvir()->recordInOutputVector(vectors["firstHopX"], t,
                                     data->getFirstHopOrigin().x);
    getEnvir()->recordInOutputVector(vectors["firstHopY"], t,
                                     data->getFirstHopOrigin().y);

    getEnvir()->recordInOutputVector(vectors["lastHopId"], t,
                                     data->getLastHopId());
    getEnvir()->recordInOutputVector(vectors["lastHopTime"], t,
                                     data->getLastHopTime().dbl());
    getEnvir()->recordInOutputVector(vectors["lastHopX"], t,
                                     data->getLastHopOrigin().x);
    getEnvir()->recordInOutputVector(vectors["lastHopY"], t,
                                     data->getLastHopOrigin().y);

  } else {
    throw cRuntimeError("%s: Expected DetourAppPacket*", getClassName());
  }
}

void IncidentRecorder::finish(cResultFilter *prev) {
  //
  //  omnetpp::cHistogram *stat = new omnetpp::cHistogram("fooBar", false);
  //  //  getEnvir()->recordStatistic(component, name, statistic, attributes)
  //  attributes.insert(std::pair<opp_string, opp_string>("fooType", "42"));

  unregisterVectors();

  //  for (auto &elem : incidentCount) {
  //    std::stringstream name;
  //    name << getResultName().c_str() << ":" << elem.first;
  //    opp_string_map attributes = getStatisticAttributes();
  //    attributes.insert(std::pair<opp_string, opp_string>(
  //        "firstOccurance:incidentDelta", elem.second.incidentDelta.str()));
  //    attributes.insert(
  //        std::pair<opp_string,
  //        opp_string>("firstOccurance:incidentStartTime",
  //                                          elem.second.incidentStartTime.str()));
  //    attributes.insert(std::pair<opp_string, opp_string>(
  //        "firstOccurance:incidentReceivedTime",
  //        elem.second.incidentReceivedTime.str()));
  //    attributes.insert(std::pair<opp_string, opp_string>(
  //        "firstOccurance:incidentOrigin", elem.second.incidentOrigin.str()));
  //    attributes.insert(std::pair<opp_string, opp_string>(
  //        "firstOccurance:lastHopOrigin", elem.second.lastHopOrigin.str()));
  //
  //    getEnvir()->recordScalar(getComponent(), name.str().c_str(),
  //                             elem.second.count,
  //                             &attributes);  // note: this is NaN if count==0
  //  }

  //  getEnvir()->recordStatistic(getComponent(), "FooBarStats", stat,
  //                              &attributes);  // note: this is NaN if
  //                              count==0
  //  delete stat;
}

}  // namespace rover
