/*
 * OsgCoordConverter.h
 *
 *  Created on: Aug 24, 2020
 *      Author: sts
 */

#pragma once

#include <omnetpp/csimplemodule.h>

#include <memory>
#include "OsgCoordinateConverter.h"
#include "inet/common/InitStages.h"

namespace rover {

class OsgCoordConverter : public omnetpp::cSimpleModule {
 public:
  OsgCoordConverter();
  virtual ~OsgCoordConverter() = default;
  virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
  virtual void initialize(int stage) override;
  virtual void handleMessage(omnetpp::cMessage*) override;

  std::shared_ptr<OsgCoordinateConverter> getConverter() const;
  bool isInitialized() const;
  void initializeConverter(std::shared_ptr<OsgCoordinateConverter> converter);

 private:
  std::shared_ptr<OsgCoordinateConverter> _converter;
};

} /* namespace rover */
