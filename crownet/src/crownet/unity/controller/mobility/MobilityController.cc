#include "MobilityController.h"

Define_Module(MobilityController);

void MobilityController::initialize() {

    module = getParentModule()->getSubmodule("unityClient");
    if (module != nullptr) {
        unityClient = check_and_cast<UnityClient*>(module);
        std::cout << "Successfully launched rocket";

    }
    cSimpleModule::initialize();
}

void MobilityController::handleMessage(cMessage *msg) {
    cSimpleModule::handleMessage(msg);
}
;

void MobilityController::sendPosition(int id,inet::Coord coord) {
    x+=1;
      std::cout <<  "MobilityController calls:" <<x << std::endl;
    unityClient->sendMessage(id,coord);
}

