
# CrowNet - Traffic simulation

CrowNet (for crowd network) is an open-source simulation environment with which
crowd management systems can be simulated.  It also provides functionalities to investiage interactions mobility behavior and mobility behavior.
In the article 'Designing mobile application messages to impact route choice: a survey and simulation study', we conduct a traffic simulation where we redirect agents to less congested routes.
Depending on the mesage design, the route choice behavior differs. We test the effect on the traffic situation in a parameter study that can be found in the directory:
```
analysis/uq/simulation_studies/route_choice_survey
```
To repeat the simulations follow the instructions below.


## Getting Started

The CrowNet project provides easy to install docker containers for these three. The containers can either be installed by the CrowNet setup script or by pulling them from the [CrowNet docker registry](https://sam-dev.cs.hm.edu/rover/crownet/container_registry).

Setting up a simulation machine with the CrowNet environment is quite simple. You simply clone this CrowNet repository, install [Docker](https://www.docker.com/) and start the three containers.

### Required Hard- and Software

The CrowNet environment requires a Linux System. We are currently using Ubuntu 18.04.2 LTS but any major distribution should do, since the required dependencies are all part of the respective Docker containers. A system with 8 GB of RAM is recommended.

### Step-by-Step Installation Instructions

Clone the CrowNet repository:
```
cd ~
git clone -b route_choice_survey https://github.com/roVer-HM/crownet.git
```

Install Docker (if not already available on your system):
```
cd crownet/scripts
source crownetenv
install_docker.sh
```

Login to docker and pull the following two docker images:
```
docker pull ghcr.io/pedestrian-dynamics-hm/message-design-and-route-choice:psychology_pub_vadere
docker pull ghcr.io/pedestrian-dynamics-hm/message-design-and-route-choice:psychology_pub_flowcontrol
```

Create a virtual Python environement by: 

```
omnetpp exec make analysis-all
```
You can find it in the directory `out/crownet_user`.

## Run the simulation
Activate the virtual enviroment:

```
source out/crownet_user/bin/activate
```
Navigate to the simulation directory:

```
cd analysis/uq/simulation_studies/route_choice_survey
```
Before running the simulations, you need to configure the Python script first.
Oben the Python script `step_2_run_simulations.py` in a text editor and adjust the absolute paths, e.g. where the simulation output should be stored.
Since the data output is ~25GB large, we recommend to check the disk space before.
Run the simulations by:
```
python3 step_2_run_simulations.py
```
Analyze and plot the results with:

```
python3 step_3_analyze_results.py
```


