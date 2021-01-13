package crownet.artery.traci;

import traci.ConnectLauncher;

simple VadereLauchner extends ConnectLauncher
{
    parameters:
        @class(crownet::VadereLauchner);
        string vadereScenarioPath;
        string vadereCachePath = default("");
        int seed = default(-1);
        bool useVadereSeed = default(false);
        string coordConverterModule = default("coordConverter"); // if geoConverter use it
}
