# Installation

Setting up a simulation machine with the CrowNet environment is quite simple. You simply clone this CrowNet repository, install [Docker](https://www.docker.com/) and start the three containers.

## Required Hard- and Software

The CrowNet environment requires a Linux System. A system with  8 GB of RAM or more is recommended.

* Install Docker Engine for your distribution based on the guidelines given by [Install Docker Engine](https://docs.docker.com/engine/install/)
* Python 3.8 is required on the host system

## Repo setup

Clone the CrowNet repository including all submodules within your home
directory. Note: The home directory will be mounted into the running containers
by default to access NED and omnetpp.ini files.

```
git clone --recurse-submodules https://github.com/roVer-HM/crownet.git
```

Note: if you are a developer, clone the gitlab repository instead. Requires credentials and configured ssh-key.  
```
git clone --recurse-submodules ssh://git@sam-dev.cs.hm.edu:6000/rover/crownet.git
```

Activate the  CrowNet environment by sourcing the setup script. This will add
environment variables to your current shell.  If you don not need a specific
container version do not change the container versions. Ensure that the public
container registry `ghcr.io/rover-hm` is selected.  
```
source setup
```

The  `crownet_info` command is no available and lists the CrowNet environment.
Including the used container registry and container version tags.  The container
registry and container versions can be configured using the files
`config/CONTAINER_REGISTRY.config` and  `config/CONTAINER_VERSION.config`. After
changing these files `source setup` must be rerun to update the environment. 


Pull the required Docker images. This may take some time due to the large size of
some of them and depending on your Internet connection:
```
./scripts/get_images
```
We recommended to include 'source ${CROWNET_HOME}/setup -i' in the startup file of your shell (~/.bashrc). 

Notes:
* The start script will mount your home directory so that it is visible inside the Docker container. If you have data not in your 
  home directory you must provide an secondary volume mount using the environment variabel `OPP_EXTERNAL_DATA_MNT`. See `config/USER_ENV.config`.
* The OMNeT++ IDE uses the workspace directory to store its preferences and development artifacts.
* Disable SELinux (Fedoara, RedHat, ...) with `sudo setenforce 0` for the installation, since it blocks X11 Forwarding. 

### For developers only
Please follow our [coding style instructions](./CodingStyle.md), and see our [recommendations](./Recommendations.md).

## Build CrowNet 

You can either build each [component individually](BuildOfIndividualSubModules.md) or use the top-level Makefile.
The simplest way to build the system is to use the top-level Makefile which we descrive in the following.
Preserving the required build-order, the top-level Makefile builds all the required components.
Simply run:

```
omnetpp exec make makefiles
omnetpp exec make make=release
```

Notes:
* the flag "release" forces "release" mode. To switch to "debug" mode, use "debug".
* the number of available processor cores are auto-detected during the build process and a sufficient number of threads is started.

That's all - now you can change to [simulations](../../crownet/simulations) and start running the simulations.

## Quick start

This quick start example demonstrates how to simply run a CrowNet simulation.
Please find more detailed information on how to run CrowNet simulations in our [documentation](../Running-a-Simulation/README.md)

### Step 1: Start the mobility container

Start the sumo and/or vadere container by executing the respective start script:
```
$> sumo
/bin/bash -c "cd /home/vm-sts; /init.sh "
Welcome to the roVer sumo Docker Container.

Using TRACI_PORT='' TRACI_GUI='false'

If you want to start sumo-gui manually, you can do this via docker exec:
  docker exec sumo sumo-gui
Listening on port 9999
```
or 
```
$> vadere
used image: sam-dev.cs.hm.edu:5023/rover/crownet/vadere:latest
/bin/bash -c "cd /home/vm-sts; /init.sh "
Welcome to the roVer vadere Docker Container.

Using TRACI_PORT='' TRACI_GUI='false' TRACI_DEBUG='' VADERE_LOG_LEVEL=''  VADERE_LOG=''

To launch vadere-gui call vadere exec vadere-gui
Vadere 1.15 (Commit Hash: 05e16522ef78086194e58fee6c713cb713faa6f7) [TraCI: VadereTraCI-20.0.2 This is a TraCI Server implementing only a small subset of TraCI Version 20]
INFO:root> vadere launcher listening on port 9998 ..
```

The container will now be listening for incoming TraCi commands and start mobiliy simulator. The 
TraCI port for sumo defaults to `9999` and for vadere to `9998`.

### Step 2: Start the omnetpp container and open workspace
Start the omnetpp container:
```
omnetpp-ide
```
Setup the Omnet++ IDE as described [here](../Running-a-Simulation/StartUp-GUI.md) and build the project in release or debug mode.

### Step 3: Run the simulation
Got to `crownet/simulations/testSim` in the project view and search for the `omentpp.ini` file.
Run the simulation by doing a right-click and selecting "Run as OMNeT++ simulation". When the simulation GUI is visible, 
select one of the following configurations and start the simulation:

1. vadere_test001 ("requires that you started the vadere container as described above")
2. sumo_crossing_peds ("requires that you started the sumo container as described above")
3. sumo_crossing_peds_cars ("requires that you started the sumo container as described above")
4. test_control00* ("requires that you started the vadere and the flowcontrol container")

Note: 
* See crownet/simulations/networks/default_configs.ini for common settings such as traci host and port setup.








