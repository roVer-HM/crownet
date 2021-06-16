package crownet.artery.traci;

import traci.ConnectLauncher;

simple SumoLauchner extends ConnectLauncher
{
    parameters:
        @class(crownet::SumoLauchner);
        string sumoCfgBase;
        int seed = default(-1); // use default random object if -1. Otherwise use set seed
}
