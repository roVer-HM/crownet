#pragma once
#include <omnetpp.h>

namespace crownet {

class UnityClient : public omnetpp::cSimpleModule{

protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage*msg) override;

public:
    void sendMessage();

};

}
