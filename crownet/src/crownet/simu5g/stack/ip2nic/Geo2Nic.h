/*
 * Geo2Nic.h
 *
 *  Created on: Aug 17, 2020
 *      Author: sts
 */

#pragma once

#include "inet/networklayer/common/NetworkInterface.h"
#include "artery/inet/InetRadioDriver.h"
#include "stack/ip2nic/IP2Nic.h"

namespace crownet {

class Geo2Nic : public ::IP2Nic {
 public:
  virtual ~Geo2Nic() = default;

 protected:
  void toStackUe(inet::Packet* datagram) override;
  void toIpUe(inet::Packet* datagram) override;

  void toStackBs(inet::Packet* datagram) override;
  void toIpBs(inet::Packet* datagram) override;

  virtual void initialize(int stage) override;

  void prepareForGeo(
      inet::Packet* datagram, const inet::Protocol* protocol);

  protected:
      const inet::Protocol* geonetProtocol;

};

} /* namespace crownet */
