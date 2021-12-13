package crownet.artery.traci;

import traci.ConnectLauncher;

simple VadereLauncher extends ConnectLauncher
{
    parameters:
        @class(crownet::VadereLauncher);
        string vadereScenarioPath;
        string vadereCachePath = default("");
        int seed = default(-1);
        bool useVadereSeed = default(false);
}
