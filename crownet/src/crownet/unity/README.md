# Unity Client
1. Collects movement information of people, vehicles and stationary after an update is received by SUMO or Vadere. <br>
2. Collects package information of people, vehicles and stationary send and received by any entity except themself.
3. Directly sends the messages to an outside server or client with the following JSON format:

- SourceID
- TargetId
- Coordinates
- ObjectType



## Activation
To activate the logging and send functionality the module needs to be added to the .ini file of the scenario.

```bash
*.visualization.mobilityVisualizer.typename ="UnityMobilityVisualizer"
*.visualization.physicalLinkVisualizer.typename = "UnityNetworkVisualizer"
*.visualization.physicalLinkVisualizer.displayLinks = true
```

On simulation start the server will try to connect to either [ECAL](https://github.com/skiunke/EventControllerAndLogger) or Unity directly.
