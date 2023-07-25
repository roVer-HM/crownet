
# Configuration of simulations

CrowNet is build on several well-documented simulation frameworks. 
Please find the following manuals to understand how a basic simulation is setup:
- [OMNeT++ Simulation Manual](https://doc.omnetpp.org/omnetpp/manual/)
- [OMNeT++ Simulation Manual](https://doc.omnetpp.org/omnetpp/manual/)
- [INET User's Guide](https://inet.omnetpp.org/docs/users-guide/)
- [INET Developer's Guide](https://inet.omnetpp.org/docs/developers-guide/)
- [VEINS tutorial](https://veins.car2x.org/tutorial/).

## Configuration files

### omnetpp.ini
Contains network specific configuration.
Every simulation in omnetpp needs a `omnetpp-ini` file as starting point. Here we will set up network specific parameters. This file may also include shared configuration via `include ../networks/default_configs.ini` command to include shared configurations.

The simulations and their `omnetpp.ini` file can be found in: `crownet/crownet/simulations`

```yaml
[Config star_wars_simulation]
network = simple_star_wars

[Config parent_config]
name = "Anakin"
last_name = "Skywalker"
bad_guy = True

[Config child_config]
extends = star_wars_simulation, parent_config
name = "Luke"
bad_guy = False
```

The square brackets signal the beginning of a new configuration. The `extends` parameter includes existing configuration into the current configuration. This configuration can be either in the same `omnetpp.ini` file or in the `default_configs.ini` file. The other lines are setting up network parameters. Reassigning a parameter to a child will override their parent parameter.

With the setup above, the `simple_star_wars` network would consist of the following parameters

```yaml
network = simple_star_wars
name = Luke
last_name = Skywalker
bad_guy = False
```

### default_configs.ini
Contains configuration shared between multiple simulations.
This file follows the same scheme as the normal omnetpp.ini file, but this file is only used to derive existing configuration. This file is located in 
`crownet/crownet/simulations/networks/default_configs.ini`.

**Note:** Simulation networks should always be in the simulation's own `omnetpp.ini` file and not in the `default_configs.ini`.

## Configuration of the mobility provider

There are two ways to consider mobility, that is, dynamic positions in the mobile networks simulation. 
1. use pre-computed mobility traces 
2. update the node position online (Simultaneous executions of Omnet++ and the mobility simulator)

Using pre-computed mobility traces reduces the complexity of the simulation: 
The mobility traces is created by a mobility simulator such as "vadere" or "sumo" beforehand and 
afterwards the communication models are run independently. 
However, if you want to model a system where the mobility behaviour depends on information received via wireless simulation, 
pre-computed mobility traces cannot be used. In this case, a coupled simulation of mobility and communication needs to be performed.

### Use pre-computed mobility traces
Include the following configuration in `omnetpp.ini`:
```
[Config bonnMotion]
extends=noTraCI
# define local coordConverter 
*.coordConverter.typename = "OsgCoordConverterLocal"
*.coordConverter.srs_code = "+proj=utm +zone=32 +ellps=WGS84 +datum=WGS84 +units=m +no_defs"
*.coordConverter.offset_x = -692298.893546436m
*.coordConverter.offset_y = -5337359.5927164m
*.coordConverter.xBound = 281.135m
*.coordConverter.yBound = 233.492m

*.bonnMotionServer.typename = "BonnMotionMobilityServer"
*.bonnMotionServer.traceFile = "path/to/trace.bonnMotion"
*.bonnMotionServer.vectorNode  = "pNode"
*.pNode[*].mobility.typename = "BonnMotionMobilityClient"
*.pNode[*].... etc.
```
Please find more information on the configuration of the BonnMotion module ![here](./use-dynamic-bonnMotion-traces.md).

### Use node positions from coupled mobility provider
If the mobility provider should be executed simultaneously, specify the mobility provider in `omnetpp.ini`.
To use the Vadere simulator as mobility provider, include the following config:
```
[Config use_vadere]
**.useVadere = true
*.traci.launcher.typename = "VadereLauncher"
*.traci.launcher.hostname = "vadere"
*.traci.launcher.port = 9998

*.traci.core.typename = "VadereCore"
*.traci.core.version = -1
*.traci.core.selfStopping = false

**.vadereScenarioPath = absFilePath("vadere/scenarios/>scenario_file_name<.scenario")
*.physicalEnvironment.config = xmldoc("vadere/scenarios/>scenario_file_name<.xml")

**.coordConverterModule = "coordConverter"
*.coordConverter.coreModule ="traci.core"
*.coordConverter.typename = "OsgCoordConverterLocal" 
*.coordConverter.srs_code = "+proj=utm +zone=32 +ellps=WGS84 +datum=WGS84 +units=m +no_defs"
*.coordConverter.offset_x = -692273.8894735108m
*.coordConverter.offset_y = -5337503.5661007995m
*.coordConverter.xBound = 164.10m # USE BOUNDS froms scenario file
*.coordConverter.yBound = 215.10m # USE BOUNDS froms scenario file

*.traci.nodes.personNode = "pNode"
*.traci.nodes.typename = "VadereNodeManager"
*.traci.nodes.personSinkModule = ".mobility"
*.traci.nodes.vehicleSinkModule = ".mobility"

*.traci.mapper.typename = "VadereModuleMapper"
*.traci.mapper.personType = "crownet.nodes.ApplicationLayerPedestrian"
*.traci.subscriptions.typename = "VadereSubscriptionManager"
```

To use the sumo simulator as mobility provider, include the following config:
```
[Config use_sumo]
*.*Node[*].useVadere = false

*.traci.launcher.sumoCfgBase = absFilePath("sumo/simple/simple_only2.sumo.cfg")
*.coordConverter.netFile = xmldoc("sumo/simple/simple.net.xml")
*.visualization.sceneVisualizer.mapFile = xmldoc("sumo/simple/simple.net.xml")
*.coordConverter.typename = "OsgCoordConverterSumo"
**.coordConverterModule = "coordConverter"

**.ignoreVehicle = false
**.ignorePersons = false

*.traci.core.typename = "Core"
*.traci.core.version = -1
*.traci.core.selfStopping = false
*.traci.launcher.typename = "SumoLauncher"
*.traci.launcher.hostname = "sumo"
*.traci.launcher.port = 9999
*.traci.launcher.seed = ${repetition}
*.traci.mapper.typename = "BasicModuleMapper"
*.traci.mapper.personType = "crownet.nodes.ApplicationLayerPedestrian"
*.traci.mapper.vehicleType = "crownet.nodes.ApplicationLayerVehicle"
*.traci.nodes.typename = "SumoCombinedNodeManager"
*.traci.nodes.personSinkModule = ".mobility"
*.traci.nodes.vehicleSinkModule = ".mobility"
*.traci.nodes.personNode = "pNode"
*.traci.nodes.vehicleNode = "vNode"
*.traci.subscriptions.typename = "BasicSubscriptionManager"
```

## Configuration of the route recommender 
The simulator `flowcontrol` generates route recommendations based on density information. 
To couple `flowcontrol`, include the following config in `omnetpp.ini`:
```
*.flowController.typename =  "crownet.control.ControlManager"
*.flowController.host = "flowcontrol"
*.flowController.port = 9997
*.flowController.coreModule = "traci.core"
```





