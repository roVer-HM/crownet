/*
 * main_test.cc
 *
 *  Created on: Sep 11, 2020
 *      Author: sts
 */
#include "main_test.h"

using namespace omnetpp;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  // the following line MUST be at the top of main()
  cStaticFlag dummy;
  // initializations
  CodeFragments::executeAll(CodeFragments::STARTUP);
  SimTime::setScaleExp(-12);

  // create a simulation manager and an environment for the simulation
//  cEnvir *env = new GtestEnv(argc, argv, new EmptyConfig());
  cEnvir *env = new GtestEnv(argc, argv, new EmptyConfig());
  cSimulation *sim = new cSimulation("simulation", env);
  cSimulation::setActiveSimulation(sim);

  int ret = RUN_ALL_TESTS();

  // deallocate registration lists, loaded NED files, etc.
  CodeFragments::executeAll(CodeFragments::SHUTDOWN);
  cSimulation::setActiveSimulation(nullptr);
  delete sim;  // deletes env as well
  return ret;
}
