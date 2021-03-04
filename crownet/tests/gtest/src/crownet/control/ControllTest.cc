/*
 * ControllTest.cc
 *
 *  Created on: Mar 1, 2021
 *      Author: vm-sts
 */

#include <functional>
#include <memory>
#include <string>

#include "main_test.h"
#include "crownet/control/ControlTraCiApi.h"


using namespace crownet;

class ControlTest : public BaseOppTest {
 public:
    ControlTest(){}
  void SetUp() override {


  }
 protected:
  ControlTraCiApi api;
};

TEST_F(ControlTest, init) {

    api.connect("flowcontrol", 9997);
    std::cout << "connected" << std::endl;
    api.handleInit();



}
