package crownet.unity.visualizer;
import inet.visualizer.base.MobilityVisualizerBase;



simple CrownetMobilityVisualizer extends MobilityVisualizerBase
{
    parameters:
        @class(crownet::CrownetMobilityVisualizer);
        string foo = default("xxx");
}
