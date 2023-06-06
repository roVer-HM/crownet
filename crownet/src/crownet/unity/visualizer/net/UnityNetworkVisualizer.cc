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
}

bool UnityNetworkVisualizer::isLinkEnd(cModule *module) const
{
}

const UnityNetworkVisualizer::LinkVisualizerBase::LinkVisualization *UnityNetworkVisualizer::createLinkVisualization(cModule *source, cModule *destination, cPacket *packet) const
{
}

void UnityNetworkVisualizer::setAlpha(const LinkVisualizerBase::LinkVisualization *linkVisualization, double alpha) const
{
}




} // namespace inet
