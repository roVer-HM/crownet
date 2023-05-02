#pragma once
#include <omnetpp.h>
#include "inet/common/geometry/common/Coord.h"
#include <mutex>
using namespace omnetpp;

class UnityClient : public cSimpleModule{

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage*msg) override;
    virtual void finish() override;

public:
    int serverSocket;
    void sendMessage(int id,inet::Coord coord);
    int x =0;
    std::mutex m_mutex;

    void test(int id,inet::Coord coord);


};


