import inet.visualizer.base.MobilityVisualizerBase;
import crownet.unity.client.UnityClient;
import inet.visualizer.contract.IMobilityVisualizer;

package crownet.unity.visualizer.mob;


module UnityMobilityVisualizer extends MobilityVisualizerBase like IMobilityVisualizer
{
    @class(crownet::UnityMobilityVisualizer);
    submodules:
        unityClient: UnityClient;

}
