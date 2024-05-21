# Run simulations headless with Python

## Prepare the virtual Python environment 

Crownet uses multiple python frameworks to create simulation studies using a
combination of multiple simulators. Use the following from the repo root directory
to create ready to use virtual environments:

```
make analysis-all
```
This will create two virtual environments `out/crownet_user` and `out/crownet_dev`.
In the first, the three packages `crownetutils`, `flowcontrol` and `suqc` are
installed based on the current branch of the respective sub modules.
The second environment (`out/crownet_dev`) only installs the respective
requirements of the three packages. Use the `out/crownet_dev` environment
in your IDE. See [this wiki page][345] how to setup a single PyCharm
project to develop in all python packages.

## Run the simulation
The simulation startup with a run-script allows testing of multiple parameters for a given simulation without executing every service singly.
This allows an easy way to test the simulation and execute simulation studies on it.

Now we can start the run-script without manually starting any container:
  * set your current working directory to `$CROWNET_HOME/crownet/simulations/testSim`
  * run the run-script with following arguments:
   `(crownet_user) python3 run_script.py sumo --opp.-c sumo_crossing_peds_cars --override-host-config --create-sumo-container`
  * another example could be:
   `(crownet_user) python3 run-script.py vadere --override-host-config --scenario-file "vadere/scenarios/test001_with_signs.scenario"`

If you want more examples of how the run_script is configured for the simulation studies, check out the
[guiding_crowds.yaml file in the fingerprint tests](../../crownet/tests/fingerprint/guiding_crowds.yml).

# sub-commands
Be aware that there are multiple sub-commands, and they are mutually exclusive, meaning you can't run an omnet simulation with both vadere and sumo at the same time.
The available commands are as follows:

|sub-command|vadere|control|sumo|omnet|
| --- | --- | --- | --- | --- |
| vadere | &#x2714; | &#x2718;| &#x2718; | &#x2718; |
| vadere-control | &#x2714;| &#x2714; | &#x2718; | &#x2718; |
| vadere-opp | &#x2714;| &#x2718; | &#x2718; | &#x2714; |
| vadere-opp-control | &#x2714;| &#x2714; | &#x2718; | &#x2714; |
| sumo | &#x2718;| &#x2718; | &#x2714; | &#x2714; |

For a more detailed explanation, be aware that both, the run-script and its sub-commands provide a `--help` argument.