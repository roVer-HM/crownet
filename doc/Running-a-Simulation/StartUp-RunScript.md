# Run simulations headless with Python

## Prepare the virtual Python environment 

CrowNet uses multiple Python frameworks to create simulation studies using a
combination of multiple simulators. Use the following from the repo root directory
to create ready-to-use virtual environments:

```
make analysis-all
```
This will create two virtual environments `out/crownet_user` and `out/crownet_dev`.
In the first, the three packages `crownetutils`, `flowcontrol` and `suqc` are
installed based on the current branch of the respective sub modules.
The second environment (`out/crownet_dev`) only installs the respective
requirements of the three packages. Use the `out/crownet_dev` environment
in your IDE. See [the IDE setup section in the Crowd Control guide](../archive/How-to-implement-and-test-crowd-control-in-CrowNet.md#run-simulations-in-an-ide) to set up a single PyCharm
project to develop in all python packages.

## Run the simulation
Running the simulation via `run_script.py` allows testing multiple parameters without starting every service manually.
This provides an easy way to test simulations and execute simulation studies.

Now you can start the run script without manually starting any container:
  * set your current working directory to `$CROWNET_HOME/crownet/simulations/testSim`
  * run the script with the following arguments:
   `(crownet_user) python3 run_script.py sumo --opp.-c sumo_crossing_peds_cars --override-host-config --create-sumo-container`
  * another example could be:
   `(crownet_user) python3 run_script.py vadere --override-host-config --scenario-file "vadere/scenarios/test001_with_signs.scenario"`

## Sub-commands
Be aware that there are multiple sub-commands, and they are mutually exclusive. This means you cannot run an OMNeT++ simulation with both Vadere and SUMO at the same time.
The available commands are as follows:

|sub-command|vadere|control|sumo|omnet|
| --- | --- | --- | --- | --- |
| vadere | &#x2714; | &#x2718;| &#x2718; | &#x2718; |
| vadere-control | &#x2714;| &#x2714; | &#x2718; | &#x2718; |
| vadere-opp | &#x2714;| &#x2718; | &#x2718; | &#x2714; |
| vadere-opp-control | &#x2714;| &#x2714; | &#x2718; | &#x2714; |
| sumo | &#x2718;| &#x2718; | &#x2714; | &#x2714; |

For a more detailed explanation, both the run script and its sub-commands provide a `--help` argument.