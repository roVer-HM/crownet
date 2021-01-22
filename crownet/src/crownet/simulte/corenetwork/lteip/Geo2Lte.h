/*
 * Geo2Lte.h
 *
 *  Created on: Aug 17, 2020
 *      Author: sts
 */

#pragma once

#include "artery/inet/InetRadioDriver.h"
#include "corenetwork/lteip/IP2lte.h"

namespace crownet {

class Geo2Lte : public ::IP2lte {
 public:
  virtual ~Geo2Lte() = default;

 protected:
  void toStackUe(inet::Packet* datagram) override;
  void toIpUe(inet::Packet* datagram) override;

  void toStackEnb(inet::Packet* datagram) override;
  void toIpEnb(inet::Packet* datagram) override;

  virtual void initialize(int stage) override;

  void prepareForGeo(
      inet::Packet* datagram,
      const inet::Protocol* protocol = &artery::InetRadioDriver::geonet);
};

} /* namespace crownet */
