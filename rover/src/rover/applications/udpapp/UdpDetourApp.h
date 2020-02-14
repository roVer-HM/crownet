/*
 * UdpDetourApp.h
 *
 *  Created on: Feb 12, 2020
 *      Author: sts
 */

#pragma once
#include <vector>

#include "inet/applications/udpapp/UdpBasicApp.h"
#include "inet/common/INETDefs.h"
using namespace inet;

namespace rover {

class UdpDetourApp : public inet::UdpBasicApp {
 public:
  UdpDetourApp() {}
  virtual ~UdpDetourApp();

 protected:
  enum DetourSelfMsgKinds { START = 1, SEND, STOP, PROPAGATE };
  virtual void initialize(int stage) override;
  virtual void handleMessageWhenUp(cMessage *msg) override;

  virtual void sendPacket() override;

  virtual void processStart() override;
  virtual void processSend() override;
  virtual void processPacket(Packet *pk) override;

  virtual void processPropagate();

  // state
  cMessage *selfMsgIncident = nullptr;

  // parameters
  std::string reason;
  int closedTarget;
  std::vector<int> alternativeRoute;
  simtime_t repeatTime;
};

} /* namespace rover */
