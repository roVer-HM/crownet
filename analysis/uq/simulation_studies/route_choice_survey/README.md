
# Simulation study of crowd guiding app

## Requirements
Clone the crownet repository.
Navigate to the project root.
Add env variables
```
cd scripts
source crownetenv
echo $CROWNET_HOME
```
Check if CROWNET_HOME points to the project root.

Install docker if not done yet:
```
./install_docker.sh
```
Pull docker images
```
./pull_images.sh
```
Navigate to the root and build virtual python envs:
```
omnetpp exec make analysis-all```
```

## Set up IDE (recommended)
Open crownet root in PyCharm and choose as virtual env: CROWNET_HOME/out/crownet_dev/bin/python3.8

Set up your PyCharm project as described here (see Run simulations in an IDE)
https://sam-dev.cs.hm.edu/rover/crownet/-/wikis/How-to-implement-and-test-crowd-control-in-CrowNet


# 1 Run a single simulation

Skip the next step, if you use an IDE. Activate virtual environment:
```
source $CROWNET_HOME/out/crownet_user/bin/activate
```

Navigate to the simulation directory
```
cd $CROWNET_HOME/crownet/simulations/route_choice_app
```

Configure the script run_script.py (see TODOs):
* choose a scenario: 'vadere/scenarios/route_choice_real_world.scenario' or 'vadere/scenarios/three_corridors.scenario'
* choose a controller: 'NoController' or 'ClosedLoop' or 'OpenLoop'


If you work with an IDE, add CROWNET_HOME as env variable in the script configuration.
Run the run_script.
The results are stored within the directory.
To visualize the results, start the vadere-postvis in a docker container and choose the *.traj file.
```
vadere-postvis
```


# 2 Run a parameter study
To start a simulation study. Navigate to
```
cd $CROWNET_HOME/analysis/uq/simulation_studies/route_choice_survey
```
If you work with an IDE, add CROWNET_HOME as env variable in the script configuration.
Run step_2_run_simulations.py

The results will be collected in the same directory.

To plot the results, run step_3_analyze_simulation_studies (TODO: fix plotting).

## Scenario file adjustments
Note that I have changed the numbering of some scenario elements and data processors.
* target: old id: 31, new id: 11
* target: new id: 11, new id: 31
* measurement area: old id: 3, new id: 2
* measurement area: old id: 5, new id: 3
* data processors: ...


## How to adjust the control algorithm
To change the control algorithm, navigate to the simulation directory
```
cd $CROWNET_HOME/crownet/simulations/route_choice_app
```
Open control.py
If you use the IDE-containers (control-ide and vadere-ide), you can debug control.py.
However, I do not recommend that for now.
Adjust the control algorithm.


## If something goes wrong
Check docker containers:
```
docker ps -a
```
Check the logging:
```
journalctl -b CONTAINER_TAG=name_of_docker_container -f
```
Stop and remove failed containers:
```
docker stop $(docker ps -a -q)
docker rm $(docker ps -a -q)
```
Retry.

## TODOs - Manuel
* choose the correct scenario file in run_script.py.
* Run a single simulation (step 1)
* Run a parameter study (step 2)

* compare your scenario file with the $CROWNET_HOME/crownet/simulations/route_choice_app/vadere/scenarios/route_choice_real_world.scenario

Come back to me so we can discuss how you have to adjust the scenario file and control algorithm...
* adjust the scenario file in $CROWNET_HOME/crownet/simulations/route_choice_app/vadere/scenarios/route_choice_real_world.scenario (make sure that measurements areas are rectangles, otherwise the script will fail) 
