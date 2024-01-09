/*
 * LteRadioDriver.h
 *
 *  Created on: Aug 14, 2020
 *      Author: sts
 */

#pragma once

#include <artery/inet/InetRadioDriver.h>
#include <artery/utility/Channel.h>
#include <inet/networklayer/common/NetworkInterface.h>
#include <omnetpp/clistener.h>

namespace crownet {

class LteRadioDriver : public artery::RadioDriverBase,
                       public omnetpp::cListener {
 public:
  virtual ~LteRadioDriver() = default;
  int numInitStages() const override;
  using artery::RadioDriverBase::initialize;
  void initialize(int stage) override;
  void handleMessage(omnetpp::cMessage*) override;

 protected:
  void receiveSignal(omnetpp::cComponent*, omnetpp::simsignal_t, double,
                     omnetpp::cObject*) override;
  void handleDataIndication(omnetpp::cMessage*);
  void handleDataRequest(omnetpp::cMessage*) override;
  void refreshDisplay() const override;

 protected:
     const inet::Protocol* geonetProtocol;

 private:
  inet::NetworkInterface* interfaceEntry = nullptr;
  int numPassedUp, numSent;
  artery::ChannelNumber channelNumber;
};

} /* namespace crownet */
