#include "toctic.h"

using namespace omnetpp;

simsignal_t sizeSignalId = TocTic::registerSignal("size");

Define_Module(TocTic);

void TocTic::initialize() {
  if (strcmp("tic", getName()) == 0) {
    cMessage *msg = new cMessage("ticTocMsg");
    send(msg, "out");
  }
}

void TocTic::handleMessage(cMessage *msg) { send(msg, "out");

EV << "TocTic is handling messages";

}
