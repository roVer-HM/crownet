# How to run simulations with Python

## System setup
Python >= 3.7 is required. We strongly recommend to use a virtual environment:

Navigate to the crownet root directory
```
cd $CROWNET_HOME

```
Create a virtual enviroment. Install the requirements
```
python3.7 -m venv .venv
source .venv/bin/activate
pip3 install -r requirements.txt
```
##  Run simulations
### Finding the correct entry-point

Each simulation is stored in a separate simulation directory.
If the simulation directory contains a `control.py` file
FOTO
crowd guiding is available in the simulation.
There are two entry-points:  
* `run_script.py`. 
*  `control.py`

It depends on the type of simulation which entry-point should be used. 
First find out which type of simulation you want to run. 
|  			   			 		 |  			 Scope 		                                                             |  			 Pedestrian movement 		 |  			 Mobile networks 		 |  			 Behavior change 		 |
|-------|----------------------------------------------------------------------|------------------------|--------------------|--------------------|
|  			 1 		  |  			 Detailed pedestrian movement in mobile networks 		                   |  			 x 		                   |  			 x 		               |  			   			 		              |
|  			 2 		  |  			 Guiding crowds using navigation apps 		                              |  			 x 		                   |  			 x 		               |  			 x 		               |
|  			 3 		  |  			 Normal crowd behavior 		                                             |  			 x 		                   |  			   			 		              |  			   			 		              |
|  			 4 		  |  			 Rerouting crowds (ideal case: perfect information, full 			reaction) 		 |  			 x 		                   |  			   			 		              |  			 x 		               |
|  			 5 		  |  			 Guiding crowds using signs 		                                        |  			 x 		                   |  			   			 		              |  			 x 		               |

Note: the `run_script.py` is entry-point for the [suq-controller](https://gitlab.lrz.de/vadere/suq-controller) which is a Python package that allows to setup and run parameter studies. 

### Run existing simulations from terminal
Open a terminal and activate the previously created virtual environment.
```
source $CROWNET_HOME/.venv/bin/activate
```
Navigate to the simulation directory, e.g. `guiding_crowds_test`
```
cd $CROWNET_HOME/crownet/simulations/guiding_crowds_test
```

The  `run_script.py` serves as entry-point. It starts the each simulator in a separate Docker container. Debugging is not available in this mode. If you need to debug through the code, move on to section *Run simulations in an IDE*.

Run the simulation
|  			   			 		 |  			 Scope 		                                  | Command	              | Comment|
|-------|-------------------------------------------|----------------------|------------|
|  			 1 		  |  			 Pedestrian movement in mobile networks 		 | `python3 run_script.py --experiment-label crownet_simulation --delete-existing-containers --create-vadere-container --config final`  | Config final in omnetpp.ini must not contain flowController application.
|  			 2 		  |  			 Guiding crowds using navigation apps 		   | `python3 run_script.py --experiment-label crownet_with-control --delete-existing-containers --create-vadere-container --with-control path/to/python-file-that-contains-control --config final_control`	                 | Config final-control in omnetpp.ini must contain flowController application. |
|  			 3 		  |  			 Normal crowd behavior 		                  | `python3 run_script.py --experiment-label normal_crowd --vadere-only --scenario-file /path/to/vadere-scenario-file` 		                 | |
|  			 4 		  |  			 Rerouting crowds	                        | `python3 run_script.py --experiment-label ideal_crowd_guiding --control-vadere-only --delete-existing-containers --create-vadere-container --with-control path/to/python-file-that-contains-control`		                 | |
|  			 5 		  |  			 Guiding crowds using signs 		        |`python3 run_script.py --experiment-label sign_management --vadere-only --scenario-file /path/to/vadere-scenario-file-with-target-changer-as-signs`		                 | |


### Run simulations in an IDE
When developing new control strategy, we recommend to use an IDE.

Start any IDE. Set the crownet root directy as project root. The project root is
```
echo $CROWNET_HOME
```
If not found automatically by the IDE, choose the virtual enviroment `crownet/.venv` as project interpreter.

We also recommend to add the following directories to the project source for navigating easily through the codebase:
* crownet/analysis/roveranalyzer
* crownet/flowcontrol
* crownet/crownet/simulations

![source](uploads/96b35ac6be1e7df1815aa7c1b98c02ad/source.png)
Note: the PYTHONPATH is set dynamically at simulation start. If the source paths are not set in the IDE, the simulation can still be executed, but file linking is not supported by the IDE.


### Running existing simulations with control

CrowNet Simulations can be run with and without control strategy. If a control strategy should be applied, a `control.py` file must be provided in the simulation directory. Existing CrowNet Simulations can be found in the CrowNet repository under
`crownet/crownet/simulations`:
![simulations](uploads/42243a20adf8d0cea32f7b9c3ee62844/simulations.png)

We will use the simple_detour_suqc_traffic simulation as example:
![sim_example](uploads/d84750ac7d415b37b0e0d4b0fd97d643/sim_example.png)


## Step 1: Test the crowd control strategy without mobile networks
In the first step, the control strategy should be tested without mobile networks. That means, that virtual pedestrians (agents) are immediately informed. This is the ideal case. The means of transmission is not modeled. Only if the strategy is successful, the developer should move on to step 2, where the information is disseminated through mobile networks.

In step 1, only the crowd simulator Vadere and the control framework FlowControl is required (VadereControl). The simulation of the mobile network part is left out. 

There are four possibilities how the VadereControl simulation can be run.

|  			 # 		   |  			 Simulation 		 |  			   			 		               |  			 Entrypoint 		    |  			 Scope 		                                   |  			 Visualization 		             |
|--------|---------------|---------------------|------------------|--------------------------------------------|------------------------------|
|  			 1-1 		 |  			 Run  			 		      |  			 container-based 		  |  			 run_script.py 		 |  			 Run existing simulations 		                |  			 ? 		                         |
|  			 1-2 		 |  			   			 		         |  			 local 		            |  			 control.py 		    |  			   			 		                                      |  			 Switch on/off: vadere-gui 		 |
|  			 1-3 		 |  			 Debug  			 		    |  			 control 		          |  			 control.py 		    |  			 Investigate control strategy 		            |  			 Switch on/off: vadere-gui 		 |
|  			 1-4 		 |  			   			 		         |  			 control + vadere 		 |  			 control.py 		    |  			 Investigate control strategy and vadere 		 |  			 Switch on/off: vadere-gui 		 |



### 1-1 Run VadereControl as black-box simulator (container-based)

#### From terminal

If not active yet, activate the previously created virtual enviroment
```
source $CROWNET_HOME/.venv/bin/activate
```
To run VadereControl as black-box, navigate to the simulation directory
```
cd $CROWNET_HOME/crownet/simulations/simple_detoure_suqc_traffic

```
Start the run_script
```
python3 run_script.py --experiment-label test --delete-existing-containers --create-vadere-container --with-control control.py --control-vadere-only --scenario $PWD/scenario002.scenario

```
The simulation output can be found in `vadere-server-output`.

#### In IDE
Optional: to overwrite the settings defined in __main__, add `--experiment-label test --delete-existing-containers --create-vadere-container --with-control control.py --control-vadere-only --scenario $PWD/scenario002.scenario` to Configuration/arguements.

Start run_script.py. 

### 1-2 Run VadereControl (locally)
If the control strategy is succesful, ... explain here why to run locally

If not active yet, activate the previously created virtual enviroment
```
source $CROWNET_HOME/.venv/bin/activate
```
Start the simulation without containers.
```
python3 control.py --port 9999 --host-name vadere --client-mode --scenario $PWD/scenario002.scenario
```



