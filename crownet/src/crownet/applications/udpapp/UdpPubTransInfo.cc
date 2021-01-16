/*
 * UdpPubTransInfo.cc
 *
 * Created on: Nov 19, 2019
 * Author: stsc
 *
 * Copyright (C) 2019 Munich University of Applied Sciences
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#include "crownet/applications/udpapp/UdpPubTransInfo.h"

#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"

using inet::L3Address;
using inet::L3AddressResolver;
using omnetpp::cStringTokenizer;

namespace crownet {

Define_Module(UdpPubTransInfo);

UdpPubTransInfo::UdpPubTransInfo() {
  // TODO(stsc) Auto-generated constructor stub
}

UdpPubTransInfo::~UdpPubTransInfo() {
  // TODO(stsc) Auto-generated destructor stub
}

void UdpPubTransInfo::processStart() {
  socket.setOutputGate(gate("socketOut"));
  const char *localAddress = par("localAddress");
  socket.bind(
      *localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(),
      localPort);
  setSocketOptions();

  const char *destAddrs = par("destAddresses");
  cStringTokenizer tokenizer(destAddrs);
  const char *token;

  while ((token = tokenizer.nextToken()) != nullptr) {
    destAddressStr.push_back(token);
    if (strstr(token, "Broadcast") != nullptr) {
      destAddresses.push_back(inet::Ipv4Address::ALLONES_ADDRESS);
    } else {
      L3Address result;
      L3AddressResolver().tryResolve(token, result);
      if (result.isUnspecified())
        EV_ERROR << "cannot resolve destination address: " << token
                 << std::endl;
      destAddresses.push_back(result);
    }
  }

  if (!destAddresses.empty()) {
    selfMsg->setKind(SEND);
    processSend();
  } else {
    if (stopTime >= SIMTIME_ZERO) {
      selfMsg->setKind(STOP);
      scheduleAt(stopTime, selfMsg);
    }
  }
}

} /* namespace crownet */
