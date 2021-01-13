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
#include "rover/applications/udpapp/detour/DetourAppPacket_m.h"
#include "rover/applications/udpapp/detour/UdpDetourApp.h"

#include "rover/artery/traci/VaderePersonController.h"
using namespace inet;

namespace rover {

class UdpDetourAppVadere : public UdpDetourApp {
 public:
  UdpDetourAppVadere();
  virtual ~UdpDetourAppVadere();

 protected:
  virtual void initialize(int stage) override;
  virtual void actOnIncident(IntrusivePtr<const DetourAppPacket> pkt) override;

  VaderePersonController* ctrl;

};  // namespace rover

} /* namespace rover */
