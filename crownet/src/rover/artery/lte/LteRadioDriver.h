/*
 * LteRadioDriver.h
 *
 *  Created on: Aug 14, 2020
 *      Author: sts
 */

#pragma once

#include <artery/inet/InetRadioDriver.h>
#include <inet/networklayer/common/InterfaceEntry.h>
#include <omnetpp/clistener.h>

namespace rover {

class LteRadioDriver : public artery::RadioDriverBase,
                       public omnetpp::cListener {
 public:
  virtual ~LteRadioDriver() = default;
  int numInitStages() const override;
  void initialize(int stage) override;
  void handleMessage(omnetpp::cMessage*) override;

 protected:
  void receiveSignal(omnetpp::cComponent*, omnetpp::simsignal_t, double,
                     omnetpp::cObject*) override;
  void handleDataIndication(omnetpp::cMessage*);
  void handleDataRequest(omnetpp::cMessage*) override;
  void refreshDisplay() const override;

 private:
  inet::InterfaceEntry* interfaceEntry = nullptr;
  int numPassedUp, numSent;
};

} /* namespace rover */
