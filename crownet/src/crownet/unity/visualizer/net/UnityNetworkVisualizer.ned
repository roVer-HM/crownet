import crownet.unity.client.UnityClient;
import inet.visualizer.contract.IPhysicalLinkVisualizer;
import inet.visualizer.base.PhysicalLinkVisualizerBase;
import inet.visualizer.contract.IPhysicalLinkVisualizer;

package crownet.unity.visualizer.net;


module UnityNetworkVisualizer extends PhysicalLinkVisualizerBase like IPhysicalLinkVisualizer
{
         parameters:
        double zIndex = default(1);
 		@class(crownet::UnityNetworkVisualizer);
    submodules:
        unityClient: UnityClient;

}
