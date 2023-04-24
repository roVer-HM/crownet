#include <omnetpp.h>
#include <string.h>

using namespace omnetpp;

class UnityClient : public cSimpleModule {
 protected:
  virtual void initialize() override;
  virtual void handleMessage(cMessage *msg) override;
};

Define_Module(UnityClient);

void UnityClient::initialize() {
    std::cout << "Client got initialized"; }

void UnityClient::handleMessage(cMessage *msg) {
  cSimpleModule::handleMessage(msg);
}
