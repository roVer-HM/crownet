/*
 * EntropyProvider.h
 *
 *  Created on: Oct 11, 2021
 *      Author: vm-sts
 */

#pragma once
#include <omnetpp.h>
#include "inet/common/geometry/Geometry_m.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"
#include <omnetpp/crng.h>

namespace crownet {


class EntropyProvider : public ::omnetpp::cOwnedObject {

public:
    virtual void initialize(cRNG* rng) = 0;
    virtual double getValue(const inet::Coord& position, const simtime_t& time, const double old_value) = 0 ;
    virtual bool selectCell(const int x, const int y, simtime_t time) = 0;
    virtual bool selectCell(const GridCellID& cellId, simtime_t time) {return selectCell(cellId.x(), cellId.y(), time);}
    virtual double getRndValue() = 0;

public:
  virtual ~EntropyProvider() = default;
  virtual EntropyProvider *dup() const override {throw omnetpp::cRuntimeError("You forgot to manually add a dup() function to class EntropyProvider");}

private:
  void copy(const EntropyProvider& other){};

protected:
  bool operator==(const EntropyProvider&) = delete;
  // make constructors protected to avoid instantiation
  EntropyProvider(const char *name=nullptr) : ::omnetpp::cOwnedObject(name){};
  EntropyProvider(const EntropyProvider& other) : ::omnetpp::cOwnedObject(other){
      copy(other);
  };

};


} // namespace
