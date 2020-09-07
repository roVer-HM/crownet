/*
 * Geo2Lte.cc
 *
 *  Created on: Aug 17, 2020
 *      Author: sts
 */

#include "rover/simulte/corenetwork/lteip/Geo2Lte.h"

#include <inet/networklayer/ipv4/Ipv4Header_m.h>
#include "rover/artery/lte/GeoNetTag_m.h"

using namespace std;
using namespace inet;
using namespace omnetpp;

namespace rover {

Define_Module(Geo2Lte);

void Geo2Lte::toStackUe(inet::Packet* pkt) {
  // if IP let parent handle it.
  auto pTag = pkt->findTag<inet::PacketProtocolTag>();
  if (pTag->getProtocol() == &inet::Protocol::ipv4) {
    IP2lte::toStackUe(pkt);
  } else if (pTag->getProtocol() == &artery::InetRadioDriver::geonet) {
    auto geoTag = pkt->getTag<rover::GeoNetTag>();
    // if needed, create a new structure for the flow

    AddressPair pair(inet::Ipv4Address(geoTag->getSrcAddrIp()),
                     inet::Ipv4Address(geoTag->getDstAddrIp()));
    if (seqNums_.find(pair) == seqNums_.end()) {
      std::pair<AddressPair, unsigned int> p(pair, 0);
      seqNums_.insert(p);
    }
    int headerSize = B(10).get();  // todo: set correct

    pkt->addTagIfAbsent<FlowControlInfo>()->setSrcAddr(geoTag->getSrcAddrIp());
    pkt->addTagIfAbsent<FlowControlInfo>()->setDstAddr(geoTag->getDstAddrIp());
    pkt->addTagIfAbsent<FlowControlInfo>()->setSrcPort(-1);
    pkt->addTagIfAbsent<FlowControlInfo>()->setDstPort(-1);
    pkt->addTagIfAbsent<FlowControlInfo>()->setSequenceNumber(seqNums_[pair]++);
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

void Geo2Lte::toStackEnb(inet::Packet* datagram) {
  // if IP let parent handle it.
  auto pTag = datagram->findTag<inet::PacketProtocolTag>();
  if (pTag->getProtocol() == &inet::Protocol::ipv4) {
    IP2lte::toStackUe(datagram);
  } else {
    error("GeoProtocol not supported from backend (Enb->UE)");
  }
}

void Geo2Lte::toIpUe(Packet* datagram) {
  // if IP let parent handle it.
  auto pTag = datagram->findTag<inet::PacketProtocolTag>();
  auto geoTag = datagram->findTag<rover::GeoNetTag>();
  if (!geoTag && pTag->getProtocol() == &inet::Protocol::ipv4) {
    IP2lte::toIpUe(datagram);
  } else {
    EV << "Geo2lte::toIpUe - message from stack: send to Geo layer" << endl;
    datagram->removeTagIfPresent<inet::PacketProtocolTag>();
    prepareForGeo(datagram);
    send(datagram, ipGateOut_);
  }
}

void Geo2Lte::toIpEnb(inet::Packet* datagram) {
  // if IP let parent handle it.
  auto pTag = datagram->findTag<inet::PacketProtocolTag>();
  if (pTag->getProtocol() == &inet::Protocol::ipv4) {
    IP2lte::toIpEnb(datagram);
  } else {
    error("GeoProtocol not supported from backend (Enb->to IP->to core net)");
  }
}

void Geo2Lte::prepareForGeo(inet::Packet* datagram,
                            const inet::Protocol* protocol) {
  // set geonet tag instead of ip4
  IP2lte::prepareForIpv4(datagram, protocol);
}

} /* namespace rover */
