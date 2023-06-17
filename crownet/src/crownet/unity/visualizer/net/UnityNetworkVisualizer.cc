#include "UnityNetworkVisualizer.h"

namespace crownet {



Define_Module(UnityNetworkVisualizer);



void UnityNetworkVisualizer::initialize(int stage){

    LinkVisualizerBase::initialize(stage);

    if (stage == inet::INITSTAGE_LOCAL){
        unityClient = UnityClient::getInstance();

    }

}



bool UnityNetworkVisualizer::isLinkStart(cModule *module) const
{
    return false;
}

bool UnityNetworkVisualizer::isLinkEnd(cModule *module) const
{
    return false;
}

const UnityNetworkVisualizer::LinkVisualizerBase::LinkVisualization *UnityNetworkVisualizer::createLinkVisualization(cModule *source, cModule *destination, cPacket *packet) const
{
}

void UnityNetworkVisualizer::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details){
LinkVisualizerBase::receiveSignal(source, signal, object, details);

printf("I got called!");
}



} // namespace inet
