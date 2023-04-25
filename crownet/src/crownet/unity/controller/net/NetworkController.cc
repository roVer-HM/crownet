#include <omnetpp.h>

using namespace omnetpp;

class NetworkController : public cSimpleModule {
 protected:
  virtual void initialize() override;
  virtual void handleMessage(cMessage *msg) override;
};
Define_Module(NetworkController);

void NetworkController::initialize() {
  cModule *module = getModuleByPath("^.UnityClient");

  std::cout << "Network Handler initialized";
}

void NetworkController::handleMessage(cMessage *msg) {
  cSimpleModule::handleMessage(msg);
}
