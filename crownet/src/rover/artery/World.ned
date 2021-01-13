package rover.artery;

import artery.storyboard.Storyboard;
import inet.environment.contract.IPhysicalEnvironment;
import inet.physicallayer.contract.packetlevel.IRadioMedium;
import traci.Manager;
import artery.inet.RSU;

network World
{
    parameters:
        bool withStoryboard = default(false);
        bool withPhysicalEnvironment = default(false);
        int numRoadSideUnits = default(0);
        traci.nodes.typename = default("VadereNodeManager");
        traci.subscriptions.typename = default("VadereSubscriptionManager");
        traci.core.typename = default("VadereCore");
        traci.launcher.typename = default("VadereLauchner");
        traci.nodes.objectSinkModule = ".mobility";
        traci.mapper.objectType = default("rover.artery.Pedestrian");
        traci.nodes.objectSinkModule = default(".mobility");
        storyboard.middlewareModule = default(".middleware");

    submodules:
        traci: Manager {
            parameters:
                @display("p=20,20");
        }

        radioMedium: <default("Ieee80211ScalarRadioMedium")> like IRadioMedium {
            parameters:
                @display("p=352,25");
                mediumLimitCache.carrierFrequency = 5.9GHz;
        }

        physicalEnvironment: <default("PhysicalEnvironment")> like IPhysicalEnvironment if withPhysicalEnvironment {
            parameters:
                @display("p=117,25");
        }

        storyboard: Storyboard if withStoryboard {
            parameters:
                @display("p=242,20");
        }

        rsu[numRoadSideUnits]: RSU {
            parameters:
                mobility.initFromDisplayString = false;
        }
}
