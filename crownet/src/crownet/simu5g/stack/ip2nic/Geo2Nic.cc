/*
 * Geo2Nic.cc
 *
 *  Created on: Aug 17, 2020
 *      Author: sts
 */

#include "Geo2Nic.h"

#include <inet/networklayer/ipv4/Ipv4Header_m.h>
#include "crownet/artery/lte/GeoNetTag_m.h"
#include <inet/linklayer/common/InterfaceTag_m.h>
#include "inet/common/IProtocolRegistrationListener.h"

using namespace std;
using namespace inet;
using namespace omnetpp;

namespace crownet {

Define_Module(Geo2Nic);

void Geo2Nic::initialize(int stage) {
    IP2Nic::initialize(stage);
    if (stage == inet::INITSTAGE_LOCAL){
        geonetProtocol = artery::getGeoNetProtocol();
    }
    else if (stage == inet::INITSTAGE_APPLICATION_LAYER){
        // register geonet protocol
        registerService(*geonetProtocol, gate("upperLayerIn"),gate("upperLayerOut"));
    }
}

void Geo2Nic::toStackUe(inet::Packet* pkt) {
  // if IP let parent handle it.
  auto pTag = pkt->findTag<inet::PacketProtocolTag>();
  if (pTag->getProtocol() == &inet::Protocol::ipv4) {
    IP2Nic::toStackUe(pkt);
  } else if (pTag->getProtocol() == geonetProtocol) {
    auto geoTag = pkt->getTag<crownet::GeoNetTag>();
    // if needed, create a new structure for the flow
    int headerSize = B(10).get();  // todo: set correct

    pkt->addTagIfAbsent<FlowControlInfo>()->setSrcAddr(geoTag->getSrcAddrIp());
    pkt->addTagIfAbsent<FlowControlInfo>()->setDstAddr(geoTag->getDstAddrIp());
    pkt->addTagIfAbsent<FlowControlInfo>()->setHeaderSize(headerSize);

    printControlInfo(pkt);

    //** Send datagram to lte stack or LteIp peer **
    send(pkt, stackGateOut_);

  } else {
    error("Only ipv4 and geo protocols supported");
  }
  // todo: handle geo, create FlowControlInfo
  //
}

void Geo2Nic::toStackBs(inet::Packet* datagram) {
  // if IP let parent handle it.
  auto pTag = datagram->findTag<inet::PacketProtocolTag>();
  if (pTag->getProtocol() == &inet::Protocol::ipv4) {
    IP2Nic::toStackUe(datagram);
  } else {
    error("GeoProtocol not supported from backend (Enb->UE)");
  }
}

void Geo2Nic::toIpUe(Packet* datagram) {
  // if IP let parent handle it.
  auto pTag = datagram->findTag<inet::PacketProtocolTag>();
  auto geoTag = datagram->findTag<crownet::GeoNetTag>();
  if (!geoTag && pTag->getProtocol() == &inet::Protocol::ipv4) {
    IP2Nic::toIpUe(datagram);
  } else {
    EV << "Geo2Nic::toIpUe - message from stack: send to Geo layer" << endl;
    datagram->removeTagIfPresent<inet::PacketProtocolTag>();
    prepareForGeo(datagram, geonetProtocol);
    send(datagram, ipGateOut_);
  }
}

void Geo2Nic::toIpBs(inet::Packet* datagram) {
  // if IP let parent handle it.
  auto pTag = datagram->findTag<inet::PacketProtocolTag>();
  if (pTag->getProtocol() == &inet::Protocol::ipv4) {
    IP2Nic::toIpBs(datagram);
  } else {
    error("GeoProtocol not supported from backend (Enb->to IP->to core net)");
  }
}

void Geo2Nic::prepareForGeo(inet::Packet* datagram,
                            const inet::Protocol* protocol) {
  // set geonet tag instead of ip4
  // add DispatchProtocolRequest so that the packet is handled by the specified protocol
  auto pDispatch_tag = datagram->addTagIfAbsent<DispatchProtocolReq>();
  pDispatch_tag->setProtocol(protocol);
  pDispatch_tag->setServicePrimitive(inet::ServicePrimitive::SP_INDICATION);
  datagram->addTagIfAbsent<PacketProtocolTag>()->setProtocol(protocol);
  // add Interface-Indication to indicate which interface this packet was received from
  datagram->addTagIfAbsent<InterfaceInd>()->setInterfaceId(networkIf->getInterfaceId());
}

} /* namespace crownet */
