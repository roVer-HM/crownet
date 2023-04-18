#include <omnetpp.h>
#include <string.h>
#include "toctic.h"

using namespace omnetpp;

class TicToc : public cSimpleModule, public cListener {
 protected:
  virtual void initialize() override;
  virtual void handleMessage(cMessage *msg) override;
  virtual void receiveSignal(omnetpp::cComponent *source,
                             omnetpp::simsignal_t signalID,
                             omnetpp::cObject *value,
                             omnetpp::cObject *details) override;

 public:
  // Register the signal
  simsignal_t lengthSignalId = registerSignal("length");
};

Define_Module(TicToc);

void TicToc::initialize() {
  // Initialize is called at the beginning of the simulation.
  // To bootstrap the process, we generate and send the first message.

  // Subscribe to the signal
  getParentModule()->subscribe(lengthSignalId, this);
  getSimulation()->getSystemModule()->subscribe(TocTic::sizeSignalId,
                                                       this);

  if (strcmp("tic", getName()) == 0) {
    cMessage *msg = new cMessage("ticTocMsg");
    send(msg, "out");
  }
}

void TicToc::handleMessage(cMessage *msg) {
  // The handleMessage() method is called whenever a message arrives
  // at the module. Here, we just send it to the other module, through
  // gate `out'. Because both `tic' and `toc' does the same, the message
  // will bounce between the two.
  send(msg, "out");  // send out the message
  EV << "TicToc is handling messages" << endl;
  // Send the payload to the signal
  emit(lengthSignalId, msg);
}

// This method is called whenever a signal is emitted
void TicToc::receiveSignal(omnetpp::cComponent *source,
                           omnetpp::simsignal_t signalID,
                           omnetpp::cObject *value, omnetpp::cObject *details) {
  EV << "Signal " << lengthSignalId << " Received Message " << value;
}
