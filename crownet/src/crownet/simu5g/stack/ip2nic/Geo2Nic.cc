/*
 * Geo2Nic.cc
 *
 *  Created on: Aug 17, 2020
 *      Author: sts
 */

#include "Geo2Nic.h"

#include <inet/networklayer/ipv4/Ipv4Header_m.h>
#include "crownet/artery/lte/GeoNetHeader_m.h"
#include <inet/linklayer/common/InterfaceTag_m.h>
#include "inet/common/IProtocolRegistrationListener.h"

using namespace std;
using namespace inet;
using namespace omnetpp;
using namespace simu5g;

namespace crownet {

Define_Module(Geo2Nic);

void Geo2Nic::initialize(int stage) {
    Ip2Nic::initialize(stage);
    if (stage == inet::INITSTAGE_LOCAL){
        geonetProtocol = artery::getGeoNetProtocol();
    }
    else if (stage == inet::INITSTAGE_APPLICATION_LAYER){
        // register geonet protocol
        registerService(*geonetProtocol, gate("upperLayerIn"),gate("upperLayerOut"));
    }
}

void Geo2Nic::toStackUe(inet::Packet *pkt) {
    // if IP let parent handle it.
    auto pTag = pkt->findTag<inet::PacketProtocolTag>();
    if (pTag->getProtocol() == &inet::Protocol::ipv4) {
        Ip2Nic::toStackUe(pkt);
    } else if (pTag->getProtocol() == geonetProtocol) {
        auto geoNetHeader = pkt->peekAtFront<GeoNetHeader>();

        auto ipFlowInd = pkt->addTagIfAbsent<IpFlowInd>();
        ipFlowInd->setSrcAddr(geoNetHeader->getSrcAddrIp());
        ipFlowInd->setDstAddr(geoNetHeader->getDstAddrIp());
        // ipFlowInd->setTypeOfService(tos);
        printControlInfo(pkt);

        auto srcAddr = geoNetHeader->getSrcAddrIp();
        auto destAddr = geoNetHeader->getDstAddrIp();
        short int tos = 0;

        // mark packet for using NR
        bool useNR;
        if (!markPacket(srcAddr, destAddr, tos, useNR)) {
            EV << "Geo2Nic::toStackUe - UE is not attached to any serving node. Delete packet."
               << endl;
            delete pkt;
            return;
        }

        // Set useNR on the packet control info
        pkt->addTagIfAbsent<TechnologyReq>()->setUseNR(useNR);

        //** Send datagram to cellular stack or LteIp peer **
        send(pkt, stackGateOut_);
    } else {
        error("Only ipv4 and geo protocols supported");
    }
}

void Geo2Nic::toStackBs(inet::Packet* datagram) {
  // if IP let parent handle it.
  auto pTag = datagram->findTag<inet::PacketProtocolTag>();
  if (pTag->getProtocol() == &inet::Protocol::ipv4) {
    Ip2Nic::toStackUe(datagram);
  } else {
    error("GeoProtocol not supported from backend (Enb->UE)");
  }
}

void Geo2Nic::toIpUe(Packet* datagram) {
  // if IP let parent handle it.
  auto pTag = datagram->findTag<inet::PacketProtocolTag>();

  // FIXME: Since Simu5G has no model of using different protocols on L3,
  //        We use the name of the packet to decide if a packet is plain
  //        IP or GeoNet. This should be implemented in a better way.
  if (std::strstr(datagram->getName(), "GeoNet") == nullptr) {   // This is an ugly hack...
    EV << "Geo2Nic::toIpUe - message " << datagram << " from stack: send to normal IPv4 layer" << endl;
    Ip2Nic::toIpUe(datagram);
  } else {
    EV << "Geo2Nic::toIpUe - message " << datagram << " from stack: send to Geo layer" << endl;
    datagram->removeTagIfPresent<inet::PacketProtocolTag>();
    prepareForGeo(datagram, geonetProtocol);
    send(datagram, ipGateOut_);
  }
}

void Geo2Nic::toIpBs(inet::Packet* datagram) {
  // if IP let parent handle it.
  auto pTag = datagram->findTag<inet::PacketProtocolTag>();
  if (pTag->getProtocol() == &inet::Protocol::ipv4) {
    Ip2Nic::toIpBs(datagram);
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
