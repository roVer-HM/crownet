/*
 * main_test.cc
 *
 *  Created on: Sep 11, 2020
 *      Author: sts
 */
#include <gtest/gtest.h>
#include <omnetpp.h>

using namespace omnetpp;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  // the following line MUST be at the top of main()
  cStaticFlag dummy;
  // initializations
  CodeFragments::executeAll(CodeFragments::STARTUP);
  SimTime::setScaleExp(-12);

  int ret = RUN_ALL_TESTS();
  // deallocate registration lists, loaded NED files, etc.
  CodeFragments::executeAll(CodeFragments::SHUTDOWN);
  return ret;
}
