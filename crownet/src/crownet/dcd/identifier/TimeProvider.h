/*
 * TimeProvider.h
 *
 *  Created on: Dec 7, 2020
 *      Author: sts
 */

#pragma once

#include <omnetpp.h>

template <typename T>
class TimeProvider {
 public:
  virtual ~TimeProvider() = default;
  virtual T now() = 0;
  virtual T zero() = 0;
};

class SimTimeProvider : public TimeProvider<omnetpp::simtime_t> {
 public:
  virtual omnetpp::simtime_t now() override { return omnetpp::simTime(); }
  virtual omnetpp::simtime_t zero() override { return omnetpp::simTime().ZERO; }
};
