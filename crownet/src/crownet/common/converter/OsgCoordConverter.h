/*
 * OsgCoordConverter.h
 *
 *  Created on: Aug 24, 2020
 *      Author: sts
 */

#pragma once

#include <omnetpp/csimplemodule.h>

#include <memory>

#include "inet/common/InitStages.h"
#include "crownet/common/converter/OsgCoordinateConverter.h"
#include "traci/Listener.h"

namespace crownet {

class OsgCoordConverterProvider {
public:
    virtual ~OsgCoordConverterProvider() = default;
    std::shared_ptr<OsgCoordinateConverter> getConverter() const { return _converter;}

protected:
 std::shared_ptr<OsgCoordinateConverter> _converter;
};


class OsgCoordConverterLocal : public omnetpp::cSimpleModule, public OsgCoordConverterProvider {
 public:
  virtual ~OsgCoordConverterLocal() = default;
  virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
  virtual void initialize(int stage) override;
  virtual void handleMessage(omnetpp::cMessage*) override;

};


class OsgCoordConverterVadere : public omnetpp::cSimpleModule, public OsgCoordConverterProvider, public traci::Listener {
    virtual ~OsgCoordConverterVadere() = default;
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(omnetpp::cMessage*) override;

    virtual void traciConnected() override;
};

class OsgCoordConverterSumo : public omnetpp::cSimpleModule, public OsgCoordConverterProvider, public traci::Listener {
    virtual ~OsgCoordConverterSumo() = default;
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(omnetpp::cMessage*) override;

    virtual void traciConnected() override;
};

} /* namespace crownet */
