/*
 * VaderePersonController.h
 *
 *  Created on: Aug 10, 2020
 *      Author: sts
 */

#pragma once

#include <artery/traci/PersonController.h>
#include "crownet/artery/traci/VadereApi.h"
#include "crownet/artery/traci/VariableCache.h"

namespace crownet {

class VaderePersonController : public traci::PersonController {
 public:
  virtual ~VaderePersonController() = default;
  VaderePersonController(const std::string& id, std::shared_ptr<API> api);
  VaderePersonController(std::shared_ptr<VaderePersonCache> cache);

  // Controller
  virtual const std::string& getTraciId() const override;
  virtual std::string getTypeId() const override;

  virtual artery::Position getPosition() const override;
  virtual artery::GeoPosition getGeoPosition() const override;
  virtual artery::Angle getHeading() const override;
  virtual Velocity getSpeed() const override;
  virtual Velocity getMaxSpeed() const override;
  virtual void setMaxSpeed(Velocity) override;
  virtual void setSpeed(Velocity) override;

  virtual Length getLength() const override;
  virtual Length getWidth() const override;

  virtual std::vector<std::string> getTargetList() const;
  virtual void setTargetList(std::vector<std::string> targetId);
  virtual void appendTarget(const std::string& targetId, bool back = true);
  virtual void setInformed(const simtime_t& start, const simtime_t& obsolte_at,
                           const std::string& data);
  virtual void send_control(std::string model, std::string metadata);

 private:
  VaderePersonController(const std::string& id,
                         std::shared_ptr<VaderePersonCache> cache,
                         std::shared_ptr<API> api);

  std::string m_id;
  std::shared_ptr<VaderePersonCache> m_cache;
  std::shared_ptr<VadereApi> m_api;
  traci::Boundary m_boundary;
};

} /* namespace crownet */
