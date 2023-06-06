#ifndef __INET_UNITYNETWORKVISUALIZER_H
#define __INET_UNITYNETWORKVISUALIZER_H

#include "inet/visualizer/base/LinkVisualizerBase.h"
#include "crownet/unity/client/UnityClient.h"

namespace crownet {


class INET_API UnityNetworkVisualizer : public inet::visualizer::LinkVisualizerBase
{

public:
    UnityClient *unityClient;
  protected:
    virtual bool isLinkStart(cModule *module) const override;
    virtual bool isLinkEnd(cModule *module) const override;

    virtual const LinkVisualization *createLinkVisualization(cModule *source, cModule *destination, cPacket *packet) const override;
    virtual void setAlpha(const LinkVisualization *linkVisualization, double alpha) const override;

    virtual void initialize(int stage) override;
};

 // namespace visualizer

} // namespace inet

#endif
