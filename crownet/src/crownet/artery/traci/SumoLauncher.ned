package crownet.artery.traci;

import traci.ConnectLauncher;

simple SumoLauncher extends ConnectLauncher
{
    parameters:
        @class(crownet::SumoLauncher);
        string sumoCfgBase;
        int seed = default(-1); // use default random object if -1. Otherwise use set seed
}
