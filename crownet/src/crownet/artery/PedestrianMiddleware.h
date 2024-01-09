/*
 * PedestrianMiddleware.h
 *
 *  Created on: Aug 11, 2020
 *      Author: sts
 */

#pragma once

#include <artery/application/Middleware.h>
#include <artery/application/MovingNodeDataProvider.h>
#include "crownet/artery/traci/VaderePersonController.h"

namespace crownet {

class PedestrianMiddleware : public artery::Middleware {
 public:
  virtual ~PedestrianMiddleware();
  void initialize(int stage) override;

 protected:
  void finish() override;
  void initializeController(omnetpp::cPar&);
  using artery::MiddlewareBase::receiveSignal;
  virtual void receiveSignal(omnetpp::cComponent*, omnetpp::simsignal_t,
                             omnetpp::cObject*, omnetpp::cObject*) override;

 private:
  VaderePersonController* mPersonController = nullptr;
  artery::MovingNodeDataProvider mDataProvider;
};

} /* namespace crownet */
