/*
 * LteRadioDriver.cc
 *
 *  Created on: Aug 14, 2020
 *      Author: sts
 */

#include "LteRadioDriver.h"

#include <mutex>
#include "artery/inet/InetRadioDriver.h"
#include "artery/networking/GeoNetIndication.h"
#include "artery/networking/GeoNetRequest.h"
#include "artery/nic/RadioDriverProperties.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/InitStages.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/chunk/cPacketChunk.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "crownet/artery/lte/GeoNetHeader_m.h"

using namespace inet;

namespace crownet {

Define_Module(LteRadioDriver);

namespace {

vanetza::MacAddress convert(const inet::MacAddress& mac) {
  vanetza::MacAddress result;
  mac.getAddressBytes(result.octets.data());
  return result;
}

inet::MacAddress convert(const vanetza::MacAddress& mac) {
  inet::MacAddress result;
  result.setAddressBytes(const_cast<uint8_t*>(mac.octets.data()));
  return result;
}

}  // namespace

int LteRadioDriver::numInitStages() const {
  return NUM_INIT_STAGES;
}

void LteRadioDriver::initialize(int stage) {
  if (stage == inet::INITSTAGE_LOCAL) {
    RadioDriverBase::initialize();
    // we were allowed to call addProtocol each time but call_once makes more
    // sense to me

    geonetProtocol = artery::getGeoNetProtocol();

    numSent = 0;
    numPassedUp = 0;
    channelNumber = artery::parseChannelNumber(par("channelNumber").stdstringValue());

    WATCH(numSent);
    WATCH(numPassedUp);

  } else if (stage == inet::INITSTAGE_NETWORK_LAYER) {
    inet::IInterfaceTable* interfaceTable =
        getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    interfaceEntry = interfaceTable->findInterfaceByName(
        par("dispatchInterfaceName").stdstringValue().c_str());

    registerProtocol(*geonetProtocol, gate("lowerLayerOut"), gate("lowerLayerIn"));
    auto properties = new artery::RadioDriverProperties();
    properties->LinkLayerAddress = convert(interfaceEntry->getMacAddress());
    properties->ServingChannel = channelNumber;
    numPassedUp++;
    indicateProperties(properties);
  }
}

void LteRadioDriver::receiveSignal(omnetpp::cComponent* source,
                                   omnetpp::simsignal_t signal, double value,
                                   cObject*) {
  // todo get some kind of ChannelLoadSignal from lte.
}

void LteRadioDriver::handleMessage(omnetpp::cMessage* msg) {
  if (msg->getArrivalGate() == gate("lowerLayerIn")) {
    handleDataIndication(msg);
  } else {
    RadioDriverBase::handleMessage(msg);
  }
}

void LteRadioDriver::handleDataRequest(omnetpp::cMessage* msg) {
  auto request =
      check_and_cast<artery::GeoNetRequest*>(msg->removeControlInfo());
  auto packet = new inet::Packet(
      "INET GeoNet over 4G/5G",
      inet::makeShared<inet::cPacketChunk>(check_and_cast<cPacket*>(msg)));

  // tag: MacAddressReq
  auto addr_tag = packet->addTag<inet::MacAddressReq>();
  addr_tag->setDestAddress(convert(request->destination_addr));
  addr_tag->setSrcAddress(convert(request->source_addr));

  // tag: DispatchProtocolReq
  auto pDispatch_tag = packet->addTagIfAbsent<inet::DispatchProtocolReq>();
  pDispatch_tag->setProtocol(geonetProtocol);
  pDispatch_tag->setServicePrimitive(inet::ServicePrimitive::SP_REQUEST);

  //tag: PacketProtocolTag
  packet->addTagIfAbsent<inet::PacketProtocolTag>()->setProtocol(geonetProtocol);
  assert(request->ether_type.host() == artery::InetRadioDriver::GEONET_ID);

  // tag: InterfaceReq
  packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(
      interfaceEntry->getInterfaceId());

  // header: GeoNetHeader (not a real header - models adaptation layer)
  auto geonetHeader = makeShared<GeoNetHeader>();
  geonetHeader->setChunkLength(B(20));

  EV << "LteRadioDriver::handleDataRequest - message " << packet << " has GeoNetTag " << endl;

  geonetHeader->setSrcAddrMac(convert(request->source_addr));
  geonetHeader->setDstAddrMac(convert(request->destination_addr));
  geonetHeader->setSrcAddrIp(interfaceEntry->getIpv4Address());
  geonetHeader->setDstAddrIp(inet::Ipv4Address::ALL_HOSTS_MCAST);

  packet->insertAtFront(geonetHeader);

  //
  //  auto up_tag = packet->addTag<inet::UserPriorityReq>();
  //  switch (request->access_category) {
  //    case vanetza::AccessCategory::VO:
  //      up_tag->setUserPriority(7);
  //      break;
  //    case vanetza::AccessCategory::VI:
  //      up_tag->setUserPriority(5);
  //      break;
  //    case vanetza::AccessCategory::BE:
  //      up_tag->setUserPriority(3);
  //      break;
  //    case vanetza::AccessCategory::BK:
  //      up_tag->setUserPriority(1);
  //      break;
  //    default:
  //      error("mapping to user priority (UP) unknown");
  //  }
  //  delete request;
  //
  numSent++;
  send(packet, "lowerLayerOut");
}

void LteRadioDriver::handleDataIndication(omnetpp::cMessage* msg) {
  auto packet = check_and_cast<inet::Packet*>(msg);
  auto geoHdr = packet->popAtFront<GeoNetHeader>();

  auto chunk = packet->peekData<inet::cPacketChunk>();
  auto gn_packet = chunk->getPacket()->dup();

  packet->removeTagIfPresent<inet::PacketProtocolTag>();

  auto* indication = new artery::GeoNetIndication();
  indication->source = convert(geoHdr->getSrcAddrMac());
  indication->destination = convert(geoHdr->getDstAddrMac());
  gn_packet->setControlInfo(indication);
  delete msg;

  numPassedUp++;
  indicateData(gn_packet);
}

void LteRadioDriver::refreshDisplay() const {
  RadioDriverBase::refreshDisplay();

  char buf[80];
  sprintf(buf, "[up|down: %d | %d]", numPassedUp, numSent);
  getDisplayString().setTagArg("t", 0, buf);
}

} /* namespace crownet */
