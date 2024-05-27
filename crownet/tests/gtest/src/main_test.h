/*
 * main_test.h
 *
 *  Created on: Dec 4, 2020
 *      Author: sts
 */

#pragma once

#include <gtest/gtest.h>
#include <omnetpp.h>
#include "inet/common/packet/Packet.h"
#include "inet/common/Units.h"
#include "inet/applications/base/ApplicationPacket_m.h"

using namespace inet;
using namespace omnetpp;

class EmptyConfig : public cConfiguration {
 protected:
  class NullKeyValue : public KeyValue {
   public:
    virtual const char *getKey() const { return nullptr; }
    virtual const char *getValue() const { return nullptr; }
    virtual const char *getBaseDirectory() const { return nullptr; }
  };
  NullKeyValue nullKeyValue;

 public:
  virtual const char *substituteVariables(const char *value) const {
    return value;
  }

  virtual const char *getConfigValue(const char *key) const { return nullptr; }
  virtual const KeyValue &getConfigEntry(const char *key) const {
    return nullKeyValue;
  }
  virtual const char *getPerObjectConfigValue(const char *objectFullPath,
                                              const char *keySuffix) const {
    return nullptr;
  }
  virtual const KeyValue &getPerObjectConfigEntry(const char *objectFullPath,
                                                  const char *keySuffix) const {
    return nullKeyValue;
  }
};

class GtestEnv : public omnetpp::cNullEnvir {
 public:
  // constructor
  GtestEnv(int ac, char **av, cConfiguration *c) : cNullEnvir(ac, av, c) {}

  // model parameters: accept defaults
  virtual void readParameter(cPar *par) {
    if (par->containsValue())
      par->acceptDefault();
    else
      throw cRuntimeError("no value for %s", par->getFullPath().c_str());
  }
  // send module log messages to stdout
  virtual void sputn(const char *s, int n) { (void)::fwrite(s, 1, n, stdout); }
};

class OppHelper {
public:
  simtime_t incrementSimTime(double incr = 1.0){
   auto sim = cSimulation::getActiveSimulation();
   simtime_t old = sim->getSimTime();
   sim->setSimTime(old+incr);
   return old;
 }

 simtime_t setSimTime(simtime_t t) {
   auto sim = cSimulation::getActiveSimulation();
   simtime_t old = sim->getSimTime();
   sim->setSimTime(t);
   return old;
 }

 Packet* build(Ptr<Chunk> content){
     auto pkt = new Packet();
     pkt->insertAtFront(content);
     return pkt;
 }

 Packet* build(b dataLength ){
     auto pkt = new Packet();
     auto content = makeShared<ApplicationPacket>();
     content->setChunkLength(dataLength);
     pkt->insertAtFront(content);
     return pkt;
 }

};

/**
 * Base test fixture to access currently active simulation.
 */
class BaseOppTest : public ::testing::Test, public  OppHelper {
 public:

};

template <typename T>
class BaseOppTestWithParameters : public ::testing::TestWithParam<T>, public OppHelper {


};
