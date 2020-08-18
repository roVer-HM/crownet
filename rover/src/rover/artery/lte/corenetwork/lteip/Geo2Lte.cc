/*
 * Geo2Lte.cc
 *
 *  Created on: Aug 17, 2020
 *      Author: sts
 */

#include "Geo2Lte.h"
#include <inet/networklayer/ipv4/Ipv4Header_m.h>
#include "rover/artery/lte/GeoNetTag_m.h"

using namespace std;
using namespace inet;
using namespace omnetpp;

namespace rover {

Define_Module(Geo2Lte);

void Geo2Lte::toStackUe(inet::Packet* datagram) {
  // if IP let parent handle it.
  auto pTag = datagram->findTag<inet::PacketProtocolTag>();
  if (pTag->getProtocol() == &inet::Protocol::ipv4) {
    IP2lte::toStackUe(datagram);
  } else if (pTag->getProtocol() == &artery::InetRadioDriver::geonet) {
    auto geoTag = datagram->getTag<rover::GeoNetTag>();
    // if needed, create a new structure for the flow

    AddressPair pair(inet::Ipv4Address(geoTag->getSrcAddrIp()),
                     inet::Ipv4Address(geoTag->getDstAddrIp()));
    if (seqNums_.find(pair) == seqNums_.end()) {
      std::pair<AddressPair, unsigned int> p(pair, 0);
      seqNums_.insert(p);
    }
    int headerSize = B(10).get();  // todo: set correct

    FlowControlInfo* controlInfo = new FlowControlInfo();
    controlInfo->setSrcAddr(geoTag->getSrcAddrIp());
    controlInfo->setDstAddr(geoTag->getDstAddrIp());
    controlInfo->setSrcPort(-1);
    controlInfo->setDstPort(-1);
    controlInfo->setSequenceNumber(seqNums_[pair]++);
    controlInfo->setHeaderSize(headerSize);
    printControlInfo(controlInfo);

    cPacket* pktToLte = new cPacket(*datagram);
    pktToLte->encapsulate(datagram);
    pktToLte->setControlInfo(controlInfo);

    //** Send datagram to lte stack or LteIp peer **
    send(pktToLte, stackGateOut_);

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
  if (pTag->getProtocol() == &inet::Protocol::ipv4) {
    IP2lte::toIpUe(datagram);
  } else {
    EV << "Geo2lte::toIpUe - message from stack: send to Geo layer" << endl;
    prepareForGeo(datagram);
    send(datagram, ipGateOut_);
  }
}

void Geo2Lte::toIpEnb(inet::Packet* datagram) {
  // if IP let parent handle it.
  auto pTag = datagram->findTag<inet::PacketProtocolTag>();
  if (pTag->getProtocol() == &inet::Protocol::ipv4) {
    IP2lte::toIpUe(datagram);
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
