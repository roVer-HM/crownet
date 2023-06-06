import inet.visualizer.base.LinkVisualizerBase;
import crownet.unity.client.UnityClient;
import inet.visualizer.contract.IPhysicalLinkVisualizer;

package crownet.unity.visualizer.net;


module UnityNetworkVisualizer extends LinkVisualizerBase like IPhysicalLinkVisualizer
{
    @class(crownet::UnityMobilityVisualizer);
    submodules:
        unityClient: UnityClient;

}
