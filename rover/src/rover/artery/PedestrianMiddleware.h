/*
 * PedestrianMiddleware.h
 *
 *  Created on: Aug 11, 2020
 *      Author: sts
 */

#pragma once

#include <artery/application/Middleware.h>
#include <artery/application/MovingNodeDataProvider.h>
#include "rover/artery/traci/VaderePersonController.h"

namespace rover {

class PedestrianMiddleware : public artery::Middleware {
 public:
  virtual ~PedestrianMiddleware();
  void initialize(int stage) override;
  void finish() override;

 protected:
  void initializeController(omnetpp::cPar&);
  void receiveSignal(omnetpp::cComponent*, omnetpp::simsignal_t,
                     omnetpp::cObject*, omnetpp::cObject*) override;

 private:
  VaderePersonController* mPersonController = nullptr;
  artery::MovingNodeDataProvider mDataProvider;
};

} /* namespace rover */
