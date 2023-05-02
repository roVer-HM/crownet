#include <omnetpp.h>
#include "inet/common/geometry/common/Coord.h"
#include "crownet/unity/client/UnityClient.h"

using namespace omnetpp;

class MobilityController: public cSimpleModule {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

public:
    void sendPosition(int id,inet::Coord coord);
    //void sendPosition();

    cModule *module;
    UnityClient *unityClient;
    int x = 0;
};

