#include <omnetpp.h>
#include <string.h>

using namespace omnetpp;

class TocTic : public cSimpleModule {
 protected:
  virtual void initialize() override;
  virtual void handleMessage(cMessage *msg) override;

 public:
  static simsignal_t sizeSignalId;
};
