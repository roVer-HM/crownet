/*
 * VaderePersonController.h
 *
 *  Created on: Aug 10, 2020
 *      Author: sts
 */

#pragma once

#include <artery/traci/MovingNodeController.h>
#include "rover/artery/traci/VadereLiteApi.h"
#include "rover/artery/traci/VariableCache.h"

namespace rover {

class VaderePersonController : public traci::MovingNodeController {
 public:
  virtual ~VaderePersonController() = default;
  VaderePersonController(const std::string& id, rover::VadereLiteApi& api);
  VaderePersonController(std::shared_ptr<VaderePersonCache> cache);

  // MovingNodeController
  virtual const std::string& getNodeId() const override;
  virtual std::string getTypeId() const override;
  virtual const std::string getNodeClass() const override;

  virtual artery::Position getPosition() const override;
  virtual artery::GeoPosition getGeoPosition() const override;
  virtual artery::Angle getHeading() const override;
  virtual Velocity getSpeed() const override;
  virtual Velocity getMaxSpeed() const override;
  virtual void setMaxSpeed(Velocity) override;
  virtual void setSpeed(Velocity) override;

  virtual Length getLength() const override;
  virtual Length getWidth() const override;

 private:
  VaderePersonController(const std::string& id,
                         std::shared_ptr<VaderePersonCache> cache,
                         traci::LiteAPI& api);

  std::string m_id;
  std::shared_ptr<VaderePersonCache> m_cache;
  VadereLiteApi& m_api;
  traci::Boundary m_boundary;
};

} /* namespace rover */
