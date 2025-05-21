# Installation

Setting up a simulation machine with the CrowNet environment is quite simple. You simply clone this CrowNet repository, install [Docker](https://www.docker.com/) and start the three containers.

## Required Hard- and Software

The CrowNet environment requires a Linux System. We are currently using Ubuntu 24.04 LTS but any major distribution should do, since the required dependencies are all part of the respective Docker containers. A system with 8 GB of RAM is recommended.

### Python

CrowNet requires identical Pyhton versions on the system and within the CrowNet containers. This will be checked and you will receive a warning
message if there is a mismatch of the installed Python versions.

### Build Essential, git and curl

CrowNet requires basic make and compilations tools. We recommend to install the build-essentials package.

```
sudo apt install build-essential git git-lfs curl
```

## Repo setup

Clone the CrowNet repository including all submodules within your home directory. 
```
cd ~
git clone --recurse-submodules https://github.com/roVer-HM/crownet.git
```
Note: if you are an internal developer at HM, clone the gitlab repository instead: 
```
cd ~
git clone --recurse-submodules ssh://git@sam-dev.cs.hm.edu:6000/rover/crownet.git
```

Install Docker (if not already available on your system):
```
cd crownet/scripts
./install_docker.sh
newgrp docker
```

Configure CrowNet by running (do not overwrite the pre-configured images tags unless you provide the respective images yourself):
```
cd ..
source setup
```
Get the required Docker images (due to the large size of some of them, this will take some time - depending on your Internet connection):
```
./scripts/get_images
```
If getting the images fails with the error message that Docker cannot be connected to, please reboot the system and try again. (This is a user rights issue,
since the user must be part of the Docker group and user rights are not 
updated in all shells with the 'newgrp' command above.)


We recommended to include 'source ${CROWNET_HOME}/setup -i' in the startup file of your shell (~/.bashrc). You can do this by editing the file '.bashrc' stored in the home directory of your user and adding the following lines:
```
# export the path to CrowNet (change according to the path in your setup!)
export CROWNET_HOME=$HOME/crownet
source ${CROWNET_HOME}/setup -i
```

Notes:
* The start script will mount your home directory so that it is visible inside the Docker container. 
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
cd $CROWNET_HOME
omnetpp exec make makefiles
omnetpp exec make MODE=release -j$(nproc)
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








