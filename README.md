# CrowNet

CrowNet (for crowd network) is an open-source simulation environment which
couples state-of-the art pedestrian locomotion models with wireless
communication models. It can be used to evaluate pedestrian communication
in urban and rural environments.


Within the CrowNet simulation environment, three open-source
simulation frameworks are coupled:
* [OMNeT++](https://omnetpp.org/) - The simulation framework used
  for modelling communication and information dissemination
* [VADERE](http://www.vadere.org/) - VADERE Crowd Simulation
* [SUMO](https://dlr.de/ts/en/sumo/) - Simulation of Urban MObility

The CrowNet project provides easy to install docker containers for these three. The containers can either be installed by the CrowNet setup script or by pulling them from the [CrowNet docker registry](https://sam-dev.cs.hm.edu/rover/crownet/container_registry).

## CrowNet Communication and Information Dissemination Simulation
The OMNeT++ simulation of communication and networking for disseminating mobility information is based on four open source simulation projects:
* [INET](https://inet.omnetpp.org/) - The INET Framework is a widely-used open-source model suite for wireless and wired networks. It includes models for all major Internet protocols.
* [Simu5G](http://simu5g.org/) - Sim5G provides a 5G and LTE/LTE-A user plane simulation model for INET and OMNeT++.
* [Artery](http://artery.v2x-research.eu/) - Artery V2X Simulation Framework
* [VEINS](https://veins.car2x.org/) - Vehicles in Network Simulation (VEINS) is an open-source vehicular network simulation framework.

Since CrowNet requires several modifications in each of these open-source projects, they are included as submodules and changes are tracked on separate branches.

## Getting Started

Setting up a simulation machine with the CrowNet environment is quite simple. You simply clone this CrowNet repository, install [Docker](https://www.docker.com/) and start the three containers.

### Required Hard- and Software

The CrowNet environment requires a Linux System. We are currently using Ubuntu 18.04.2 LTS but any major distribution should do, since the required dependencies are all part of the respective Docker containers. A system with 8 GB of RAM is recommended.

### Step-by-Step Installation Instructions

Clone the CrowNet repository including all submodules within your home directory:
```
cd ~
git clone --recurse-submodules ssh://git@sam-dev.cs.hm.edu:6000/rover/crownet.git
```

Install Docker (if not already available on your system):
```
cd crownet/scripts
source crownetenv
install_docker.sh
```

Pull the required Docker images (due to the large size of some of them, this will take some time - depending on your Internet connection):
```
pull_images.sh
```

It is also recommended to include the script 'crownet/scripts/crownetenv' in the startup file
of your shell, e.g. by adding 'source $HOME/crownet/scripts/crownetenv' at the end of ~/.bashrc. This will
include the CrowNet scripts in the search path, allowing you to start the containers easily
by simply typing their name.

If you use a Linux distribution which enables SELinux (Fedoara, RedHat, ...) the X11 Forwarding is blocked.
For now disable SELinux with `sudo setenforce 0` which will switch SELinux in Persmissive mode. After a
reboot SELinx will be enabled again.

Now you have a completely installed simulation system. Simply start the container which you want to use, e.g. by:

```
omnetpp-ide
```
or
```
vadere
```

Note:
* The start script will mount your home directory so that it is visible inside the Docker container.
* The OMNeT++ IDE uses the workspace directory to store its preferences and development artifacts.

# Information about omnet and omnet container

Information about omnet can be found in https:/ometpp.org/download. In the subfolder doc documenation can be found. The command
```
opp_run
```
used in these documentations needs to be replaced by
```
omnetpp exec opp_run
```

# Configure the Eclipse-Environment for a certain project
The eclipse environment OMNeT++ IDE uses a workspace directory to store its preferences and development artifacts. Do the following steps to create and save a project.
## Step 1: Create folder for meta-files
Create a folder in your repository
```
mkdir omnetpp-ws
```
## Step 2: Start the omnetpp container and import the subprojects
Start the omnetpp container:
```
omnetpp-ide
```
## Step 3: Import project with sub modules
Choose File>Import>General>Existing projects> and import following folders:
* inet4
* crownet
* simu5g
* artery

When importing the folder veins only import the modules 1 (veins) and 3 (inet).
* veins

## Step 4: Close and restart the Environment
Choose the folder omnetpp-ws to see the project which was just created.


# Running Your First Coupled Simulation


## Step 1: Start the mobility container

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

## Step 2: Start the omnetpp container and open workspace
Start the omnetpp container:
```
omnetpp-ide
```

Open the workspace as described in **Configure the Eclipse-Environment for a certain project** and build the project in
relase or debug mode.


## Step 3: Run the simulation
Got to `crownet/simulations/testSim` in the project view and search for the `omentpp.ini` file.
Run the simulation by doing a right-click and selecting "Run as OMNeT++ simulation". When the simulation GUI is visible, 
select one of the following configuration and start the simulation:

1. vadere_test001 (requires vadere)
2. sumo_crossing_peds (requires sumo)
3. sumo_crossing_peds_cars (requires sumo)
4. test_control00* (requires vadere **and** control) tbd!

_Note: See crownet/simulations/networks/default_configs.ini for common settings such as traci host and port setup._

## Step 4: Create and Run Own Coupled Simulations
Now you are ready to create and run your own coupled simulations. Good starting points are the [OMNeT++ Simulation Manual](https://doc.omnetpp.org/omnetpp/manual/), [OMNeT++ Simulation Manual](https://doc.omnetpp.org/omnetpp/manual/), [INET User's Guide](https://inet.omnetpp.org/docs/users-guide/), [INET Developer's Guide](https://inet.omnetpp.org/docs/developers-guide/), and the [VEINS tutorial](https://veins.car2x.org/tutorial/).

*TODO: Create CrowNet Tutorial and name it as primary reference here*

# Working with the Command Line (instead of using the OMNeT++ IDE)
If you prefer to work on the command line instead of using the graphical IDE of OMNeT++, you can use the "exec" option
of the omnetpp Docker container. It allows to execute an arbitrary command within the container.

## Building all CrowNet Components using the Top-Level Makefile
The simplest way to build the system is to use the top-level Makefile. Preserving the required build-order,
it builds all the required models. Note that it also auto-detects the number of available processor cores and
starts a sufficient number of threads.

Therefore, building all models in one step is quite simple:

```
omnetpp exec make makefiles
omnetpp exec make
```

If you want to build specifically the "release" or "debug" versions, add a MODE parameter, i.e.
```
omnetpp make MODE=debug
```

That's all - now you can change to the "simulations" subfolder and start running the simulations.

## Individually building the models
Instead of using the top-level Makefile, the models can also be built individually. Therefore, one must
first switch to the model subdirectory and than execute make locally.


*Example: Building the INET and Simu5G Frameworks*

Assuming that you have added the "omnetpp" script to your search path for executables and cloned the CrowNet project with all its submodules (see installation instructions above), you can build the INET and Simu5G models by the following commands:
```
cd inet
omnetpp exec make makefiles
omnetpp exec make -j4 MODE=release
omnetpp exec make -j4 MODE=debug
cd ..

cd simu5g
omnetpp exec make makefiles
omnetpp exec make -j4

```


*Example: Building the veins (vehicles in network simulation) projects*

The veins framework itself can be built similar to the previously mentioned components. However, it is recommended to create
the makefiles by calling the *configure* script:
```
cd veins
omnetpp exec ./configure
omnetpp exec make -j4
```

Veins includes of several subprojects - for CrowNet the veins_inet project is required. It needs to have access to the folder where INET 4 is located. The location is specified by the option `--with-inet=../../../inet4` of the *configure* script:

```
cd subprojects/veins_inet
omnetpp exec ./configure --with-inet=../../../inet4
omnetpp exec make -j4
cd ../../..
```

## Build Python Simulation Environment

Crownet uses multiple python frameworks to create simulation studies using a
combination of multiple simulators. Use the following to create ready to use
virtual environments:

```
omnetpp exec make analysis-all
```
This will create two virtual environments `out/crownet_user` and `out/crownet_dev`.
In the first, the three packages `roveranalyzer`, `flowcontrol` and `suqc` are
installed based on the current branch of the respective sub modules.
The second environment (`out/crownet_dev`) only installs the respective
requirements of the three packages. Use the `out/crownet_dev` environment
in your IDE. See [this wiki page][345] how to setup a single PyCharm
project to develop in all python packages.


[345]: ../../wikis/How-to-implement-and-test-crowd-control-in-CrowNet

# Additional Installation Steps (optional)
## Start SSH agent automatically
 When logging in into your profile, you are asked for your passphrase for your key:
 ```
eval `ssh-agent`
SSH_ASKPASS='ssh-askpass'
ssh-add ~/.ssh/id_rsa
```
with id_rsa as your private key.

# Coding Style

To ensure a standardized code style, we use the Google C++ Style Guide.
CppStyle is our recommended Eclipse Plugin. It uses the Formatter [clang-format](https://clang.llvm.org/docs/ClangFormat.html) and the Style Checker [cpplint.py](https://github.com/cpplint/cpplint).

## C++

### Installation

Within the omnetpp Docker container, [clang-format](https://clang.llvm.org/docs/ClangFormat.html) and the Style Checker [cpplint.py](https://github.com/cpplint/cpplint) are already installed.

If you additionally want to guarantee that only correctly formatted code is committed, you can set-up a git pre-commit hook
which checks the files you are about to commit. A suitable script is available within the `scripts` subdirectory. Execute:

```
user@host:~/crownet$ scripts/git-format/git-pre-commit-format install
```

If your are using the (recommended) execution of the simulation based on the provided Docker containers, you are done now. In order to disable the style check for individual projects (for example containing legacy code or code of other projects not conforming to the style guide), go to
**Project properties -> C/C++ General -> Formatter** and disable the style checks.


If the omnetpp Docker container is *not* used, the style check tools can manually be installed by the commands in the following subsections.

#### Manual Installation of cpplint.py
```
pip3 install cpplint
```

#### Manual Installation of clang-format for UNIX systems

You can install it through the package manager

For example with Ubuntu:
```
sudo apt-get install clang-format
```
Other ways installing it are building it from source or extracting it from the LLVM toolchain
and copying the bin/clang-format into your PATH.


#### Manual Installation of clang-format for Windows

There is an installer for Windows: https://llvm.org/builds/
(untested)

### Configure CppStyle

To configure CppStyle globally, go to **Preferences -> C/C++ -> CppStyle** dialog.

To configure CppSytle for a C/C++ project, go to **Project properties -> CppStyle** dialog.

To enable CppStyle(clang-format) as default C/C++ code formatter, go to **Preferences -> C/C++ -> Code Style -> Formatter** page and switch **"Code Formatter"** from **[built-in]** to **"CppStyle (clang-format)"**

To enable CppStyle(clang-format) as C/C++ code formatter for a project, go to **Project properties -> C/C++ General -> Formatter** page and switch **"Code Formatter"** from **[built-in] **to **"CppStyle (clang-format)"**

### How to use CppStyle

#### Style Check with cpplint.py
By pressing the  **Run C/C++ Code Analysis** when you right-click a file, cpplint.py will check the file and display differences in the editor.
In the Properties of CppStyle an automatic analysis can be set to trigger when saving the file.

#### Formatting Code with clang-format
The whole file or marked code can be formated using the shortcut **Command + Shift + f** on MacOS or **Ctrl + Shift + f** on Linux and other systems.

Further information: https://github.com/wangzw/CppStyle, http://www.cppstyle.com/

## Python

Use [Black](https://github.com/psf/black) Python code formatter.

### Installation

```
pip install black
```
For IDE support see Black [Editor integration](https://black.readthedocs.io/en/stable/editor_integration.html).

ToDo: add git hook to check formatting of python files.
