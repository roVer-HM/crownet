## Requirements

* crownet with all sub-projects is installed (DEBUG|RELEASE mode)
  * [getting started](todo: link to main readme.md)
  * [build-python-simulation-environment](todo: link to main readme.md)
  * https://sam-dev.cs.hm.edu/rover/crownet#getting-started
* crownet python virtual environments are available
  * https://sam-dev.cs.hm.edu/rover/crownet#build-python-simulation-environment

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

### control&#46;py
Contains configuration and the controller setup for the mobility provider. Make sure the provider is listening for the correct hostname and port. These setting can be changed in the settings array (`settings=['','', ...]`) and have to match the simulation settings.
```python
# control.py
# frequently used values:
settings = [
"--port", "9997",
"--host-name", "0.0.0.0"
]
```
To start the mobility provider, you need to run the `contol.py` script in the `control-ide` editor. 
Most of the time it is enough to run the script without any arguments, but additional arguments might be required in some cases. 
For mor information please check the docs of the [roveranalyzer](https://sam-dev.cs.hm.edu/rover/roveranalyzer) or call the tool with the `--help` argument.


### Omnet IDE
The default IDE to develop omnetpp simulations.
If started with the `omnetpp-ide` command, simulations can be started and watched inside this window. 

Can also be used for a graphical view of the `.ned` files.