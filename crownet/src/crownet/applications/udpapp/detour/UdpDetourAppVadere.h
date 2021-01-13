/*
 * UdpDetourApp.h
 *
 *  Created on: Feb 12, 2020
 *      Author: sts
 */

#pragma once
#include <vector>

#define FSM_DEBUG  // enables debug output from FSMs
#include <omnetpp.h>
#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/INETDefs.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/mobility/base/MobilityBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "crownet/applications/udpapp/detour/DetourAppPacket_m.h"
#include "crownet/applications/udpapp/detour/UdpDetourApp.h"

#include "crownet/artery/traci/VaderePersonController.h"
using namespace inet;

namespace crownet {

class UdpDetourAppVadere : public UdpDetourApp {
 public:
  UdpDetourAppVadere();
  virtual ~UdpDetourAppVadere();

 protected:
  virtual void initialize(int stage) override;
  virtual void actOnIncident(IntrusivePtr<const DetourAppPacket> pkt) override;

  VaderePersonController* ctrl;

};  // namespace crownet

} /* namespace crownet */
