#ifndef __INET_UNITYNETWORKVISUALIZER_H
#define __INET_UNITYNETWORKVISUALIZER_H

#include "inet/visualizer/canvas/base/LinkCanvasVisualizerBase.h"
#include "crownet/unity/client/UnityClient.h"

namespace crownet {


class INET_API UnityNetworkVisualizer : public inet::visualizer::LinkCanvasVisualizerBase
{

public:
    UnityClient *unityClient;
  protected:
    virtual bool isLinkStart(cModule *module) const override;
    virtual bool isLinkEnd(cModule *module) const override;
    virtual const LinkVisualization *createLinkVisualization(cModule *source, cModule *destination, cPacket *packet) const override;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
    virtual void initialize(int stage) override;
};

 // namespace visualizer

} // namespace inet

#endif
