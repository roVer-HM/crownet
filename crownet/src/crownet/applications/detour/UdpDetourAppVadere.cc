/*
 * UdpDetourAppVadere.cpp
 *
 *  Created on: Feb 12, 2020
 *      Author: sts
 */

#include "crownet/applications/detour/UdpDetourAppVadere.h"
#include <artery/traci/ControllableObject.h>

#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"
#include "crownet/applications/detour/DetourAppPacket_m.h"


using namespace inet;
using omnetpp::cStringTokenizer;

namespace crownet {
Define_Module(UdpDetourAppVadere);

UdpDetourAppVadere::UdpDetourAppVadere() {}

UdpDetourAppVadere::~UdpDetourAppVadere() {}

void UdpDetourAppVadere::initialize(int stage) {
  UdpDetourApp::initialize(stage);
  if (stage == INITSTAGE_LOCAL) {
      //
  } else if (stage == INITSTAGE_APPLICATION_LAYER) {
    auto mobility = inet::getModuleFromPar<ControllableObject>(
        par("mobilityModule"), inet::getContainingNode(this));
    ctrl = mobility->getController<VaderePersonController>();

    // record internal identifier for node.
    std::string exId = ctrl->getTraciId();
    try {
      recordScalar("externalId", std::stod(exId));
    } catch (std::invalid_argument const &e) {
      throw cRuntimeError("Cannot convert given id '%s' to long", exId.c_str());
    } catch (std::out_of_range const &e) {
      throw cRuntimeError("Given id '%s' out of range", exId.c_str());
    }
  }
}

void UdpDetourAppVadere::actOnIncident(
    IntrusivePtr<const DetourAppPacket> pkt) {
  // inform mobility provider about received information
  ctrl->setInformed(simTime().dbl(), -1.0,
                    std::string(pkt->getIncidentReason()));


  // check and act if needed.
  std::string blocked = std::string(pkt->getClosedTarget());
  std::vector<std::string> targetLists = ctrl->getTargetList();
  if (std::find(targetLists.begin(), targetLists.end(), blocked) !=
      targetLists.end()) {
    // blocked target found on traget list.
    std::vector<std::string> newTargetlist{};
    for (int i = 0; i < pkt->getAlternativeRouteArraySize(); i++) {
      newTargetlist.push_back(pkt->getAlternativeRoute(i));
    }
    ctrl->setTargetList(newTargetlist);
  }
}

} /* namespace crownet */
