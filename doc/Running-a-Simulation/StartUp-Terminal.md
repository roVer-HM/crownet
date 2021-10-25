## StartUp with Terminal
In order for the simulation to run headless, make sure the mobility container and the flowcontrol container are both running. Simply run the simulation with the following steps:

  * set your current working directory to `$CROWNET_HOME/crownet/simulations/testSim`
  * start the flowcontrol-container
    * terminal-1: `control exec "python3 control_1.py"`
  * start a mobility container (sumo|vadere):
    * terminal-2: `sumo`
    * terminal-2: `vadere`
  * run the simulation
    * terminal-3: `omnetpp exec ../../src/run_crownet -c sumo_crossing_peds -u Cmdenv`

## Validation
### omnet
Our command `omnetpp exec ../../src/run_crownet -c sumo_crossing_peds -u Cmdenv` runs a wrapper for the omnet executable, so it is not necessary to specify a specific version (debug | release). The two arguments we are passing in are rerouted to the omnet executable, where `-c` specifies the configuration name and `-u` the environment for the simulation. 

For more details about the command line interface of the omnet executable type `omnetpp exec ../../src/run_crownet -h`

After running the simulation in the headless mode, the omnet container should have an output similar to this:
```
Running simulation...
** Event #0   t=0   Elapsed: 3.5e-05s (0m 00s)  0% completed  (0% total)
     Speed:     ev/sec=0   simsec/sec=0   ev/simsec=0
     Messages:  created: 66   present: 66   in FES: 8
     ...
     ...
     ...
End.
Container terminated.
```
This is the command line output of the simulation. Be aware that not all simulation steps or events are displayed.

In addition to the command line output, the simulation output files can be found in `$CROWNET_HOME/crownet/simulations/testSim/results/sumo_crossing_peds_<timestamp>`

In our case it contains a file called `vars_rep_0.sca`, a scalar-recording. This file contains multiple scalar results that happened during the simulation.

### sumo
If we check the sumo output, we see, that for our simulation, there was a connection from omnet that has been mapped to the port `37899`.
```
Starting SUMO (sumo -c example_peds.sumo.cfg) on port 37899, seed 1123468937
Done running SUMO
```

### flowcontrol
If there is no debug output in the control script, there will be no further information from this service. Our terminal just replied:
```
start...
```

If prompted output inside the flowcontrol service (f.e. print-statement) it would appear in this window.