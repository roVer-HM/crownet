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
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "crownet/common/converter/OsgCoordinateConverter.h"
#include "crownet/common/crownet_m.h"
#include "traci/Listener.h"

namespace crownet {

class OsgCoordConverterProvider : public inet::IGeographicCoordinateSystem {
public:
    virtual ~OsgCoordConverterProvider() = default;
    std::shared_ptr<OsgCoordinateConverter> getConverter() const { return _converter;}

    virtual inet::GeoCoord getScenePosition() const override {return scenePosition;}
    virtual inet::Quaternion getSceneOrientation() const override {return sceneOrientation;}

protected:
 std::shared_ptr<OsgCoordinateConverter> _converter;
  inet::GeoCoord scenePosition = inet::GeoCoord::NIL;
  inet::Quaternion sceneOrientation = inet::Quaternion::NIL;

};


class OsgCoordConverterLocal : public omnetpp::cSimpleModule,
                               public OsgCoordConverterProvider {
 public:
  virtual ~OsgCoordConverterLocal() = default;
  virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
  virtual void initialize(int stage) override;
  virtual void handleMessage(omnetpp::cMessage*) override;


  virtual inet::Coord computeSceneCoordinate(const inet::GeoCoord& geographicCoordinate) const override;
  virtual inet::GeoCoord computeGeographicCoordinate(const inet::Coord& sceneCoordinate) const override;
};


class OsgCoordConverterVadere : public OsgCoordConverterLocal,
                                public traci::Listener{
public:
    virtual ~OsgCoordConverterVadere() = default;
    virtual void initialize(int stage) override;

    virtual void traciConnected() override;
};

class OsgCoordConverterSumo : public OsgCoordConverterLocal {
public:
    virtual ~OsgCoordConverterSumo() = default;
    virtual void initialize(int stage) override;
private:
    std::vector<double> parse(const std::string& input, const int count) const;

};

} /* namespace crownet */
