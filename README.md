# roVer Main

Project collecting general documentation and tooling for the roVer project.

The roVer simulation environment is composed of three simulation frameworks:
* [OMNeT++](https://omnetpp.org/) - The simulation framework used
  for modelling communication and information dissemination
* [VADERE](http://www.vadere.org/) - VADERE Crowd Simulation
* [SUMO](https://dlr.de/ts/en/sumo/) - Simulation of Urban MObility

The roVer project provides easy to install docker containers for these three. The containers can either be installed by the roVer setup script or by pulling them from the [roVer docker registry](https://sam-dev.cs.hm.edu/rover/rover-main/container_registry).

## roVer Communication and Information Dissemination Simulation
The OMNeT++ simulation of communication and networking for disseminating mobility information is based on three open source simulation projects:
* [INET](https://inet.omnetpp.org/) - The INET Framework is a widely-used open-source model suite for wireless and wired networks. It includes models for all major Internet protocols.
* [VEINS](https://veins.car2x.org/) - Vehicles in Network Simulation (VEINS) is an open-source vehicular network simulation framework.
* [SimuLTE](http://simulte.com/) - SimuLTE provides a LTE/LTE-A user plane simulation model for INET and OMNeT++.

Since the roVer projects requires several modifications in each of these three open-source projects, they are included as submodules and changes are tracked on separete branches.

## Getting Started

Setting up a simulation machine with the roVer environment is quite simple. You simply clone this roVer main repository, install [Docker](https://www.docker.com/) and start the three containers.

### Required Hard- and Software

The roVer environment requires a Linux System. We are currently using Ubuntu 18.04.2 LTS but any major distribution should do, since the required dependencies are all part of the respective Docker containers. A system with 8 GB of RAM is recommended.

### Step-by-Step Installation Instructions

Clone the roVer repository including all submodules within your home directory:
```
cd ~
git clone --recurse-submodules ssh://git@sam-dev.cs.hm.edu:5022/rover/rover-main.git
```

Install Docker (if not already available on your system):
```
cd rover-main/scripts
source roverenv
install_docker.sh
```

Pull the required Docker images (due to the large size of some of them, this will take some time - depending on your Internet connection):
```
pull_images.sh
```

It is also recommended to include the script 'rover-main/scripts/roverenv' in the startup file
of your shell, e.g. by adding 'source $HOME/rover-main/scripts/roverenv' at the end of ~/.bashrc. This will
include the rover scripts in the search path, allowing you to start the containers easily
by simply typing their name.

If you use a Linux distribution which enables SELinux (Fedoara, RedHat, ...) the X11 Forwarding is blocked.
For now disable SELinux with `sudo setenforce 0` which will switch SELinux in Persmissive mode. After a
reboot SELinx will be enabled again. 

Now you have a completely installed simulation system. Simply start the container which you want to use, e.g. by:
```
omnetpp
```
or
```
vadere
```

Note: The start script will mount your home directory so that it is visible inside the Docker container.


# Running Your First Coupled Simulation

*Note: As a first test for testing coupled mobility and mobile node simulation, we are simply running the
VEINS example simulation in the roVer containers. This example will be updated to an example illustrating pedistrian mobility as soon as the required TraCI interfaces have been implemented.*

In this example, we will use the sumo container for simulating the mobility and the omnetpp container for simulating mobile communication (as within the VEINS Erlangen example).

## Step 1: Start the sumo container
Start the sumo container by executing the sumo start script:
```
sumo
```
The container will now be listening for incomming TraCi commands and start sumo when a new client connects.

## Step 2: Start the omnetpp container and import the subprojects
Start the omnetpp container:
```
omnetpp
```

Open a (new) OMNeT++ workspace (path should be within your home directory!), **do not** import the examples and **do not** import the INET framework. Instead, you need to import the INET and VEINS projects within the 'rover-main' folder that have been created when cloning the 'rover-main' repository. (Import of these projects is done via File->Import->General->Existing Project into workspace.) Wait until the C++ indexer has completed its work (takes some minutes). Build both projects (takes some more minutes...).


## Step 3: Run the simulation
Within the VEINS project, locate the file 'omnetpp.ini' within the 'examples/veins' folder. Run the simulation by doing a right-click and selecting "Run as OMNeT++ simulation". When the simulation GUI is visible, select a configuration and start the simulation. The sumo container will be connected and start a sumo instance automatically.

*Note: Currently there seems to be a bug which leads to a black window in the configuration-selection dialog
the first time the simulation is started. Workaround: Simply close the dialog and rebuild the network (File->Setup a configuration...).

## Step 4: Create and Run Own Coupled Simulations
Now you are ready to create and run your own coupled simulations. Good starting points are the [OMNeT++ Simulation Manual](https://doc.omnetpp.org/omnetpp/manual/), [OMNeT++ Simulation Manual](https://doc.omnetpp.org/omnetpp/manual/), [INET User's Guide](https://inet.omnetpp.org/docs/users-guide/), [INET Developer's Guide](https://inet.omnetpp.org/docs/developers-guide/), and the [VEINS tutorial](https://veins.car2x.org/tutorial/).

*TODO: Create roVer Tutorial and it as primary reference here*

_Important note:_ Since within the roVer project we run the mobility simulation and the network simulation in separate containers, remember to update the respective ``omnetpp.ini`` to refer to the container running the mobility simulation - ``localhost`` *will not work*. This can easily be done by adapting the ``*.manager.host`` parameter.

Example: Connect to sumo within the sumo container
```
##########################################################
#            TraCIScenarioManager parameters             #
##########################################################
*.manager.updateInterval = 1s
# *.manager.host = "localhost"
*.manager.host = "sumo"
*.manager.port = 9999

```

# Working with the Command Line (instead of using the OMNeT++ IDE)
If you prefer to work on the command line instead of using the graphical IDE of OMNeT++, you can use the "exec" option
of the omnetpp Docker container. It allows to execute an arbitrary command within the container.

## Example: Building the INET and SimuLTE Frameworks
Assuming that you have added the "omnetpp" script to your search path for executibles and cloned the roVer with all its submodules (see installation instructions above), you can build the INET and SimuLTE models by the following commands:
```
cd inet
omnetpp exec make makefiles
omnetpp exec make -j4
cd ..

cd simulte
omnetpp exec make makefiles
omnetpp exec make -j4

```

## Example: Building the veins (vehicles in network simulation) projects
The veins framework itself can be built similar to the previously mentioned components. However, it is recommended to create
the makefiles by calling the *configure* script:
```
cd veins
omnetpp exec ./configure
omnetpp exec make -j4
```

Veins includes of several subprojects - for roVer the veins_inet project is required. It needs to have access to the folder where INET 4 is located. The location is specified by the option `--with-inet=../../../inet4` of the *configure* script:

```
cd subprojects/veins_inet
omnetpp exec ./configure --with-inet=../../../inet4
omnetpp exec make -j4
cd ../../..
```
# Coding Style

To ensure a standardized code style, we use the Google C++ Style Guide. 
CppStyle is our recommended Eclipse Plugin. It uses the Formatter [clang-format](https://clang.llvm.org/docs/ClangFormat.html) and the Style Checker [cpplint.py](https://github.com/cpplint/cpplint).

## Installation

Within the omnetpp Docker container, [clang-format](https://clang.llvm.org/docs/ClangFormat.html) and the Style Checker [cpplint.py](https://github.com/cpplint/cpplint) are already installed. 

If you additionally want to guarantee that only correctly formatted code is committed, you can set-up a git pre-commit hook 
which checks the files you are about to commit. A suitable script is available within the `scripts` subdirectory. Execute:

```
user@host:~/rover-main$ scripts/git-format/git-pre-commit-format install
```

If your are using the (recommended) execution of the simulation based on the provided Docker containers, you are done now. In order to disable the style check for individual projects (for example containing legacy code or code of other projects not conforming to the style guide), go to
**Project properties -> C/C++ General -> Formatter** and disable the style checks.


If the omnetpp Docker container is *not* used, the style check tools can manually be installed by the commands in the following subsections.

### Manual Installation of cpplint.py
```
pip3 install cpplint
```

### Manual Installation of clang-format for UNIX systems

You can install it through the package manager

For example with Ubuntu:
```
sudo apt-get install clang-format
```
Other ways installing it are building it from source or extracting it from the LLVM toolchain
and copying the bin/clang-format into your PATH.


### Manual Installation of clang-format for Windows

There is an installer for Windows: https://llvm.org/builds/
(untested)

## Configure CppStyle

To configure CppStyle globally, go to **Preferences -> C/C++ -> CppStyle** dialog.

To configure CppSytle for a C/C++ project, go to **Project properties -> CppStyle** dialog.

To enable CppStyle(clang-format) as default C/C++ code formatter, go to **Preferences -> C/C++ -> Code Style -> Formatter** page and switch **"Code Formatter"** from **[built-in]** to **"CppStyle (clang-format)"**

To enable CppStyle(clang-format) as C/C++ code formatter for a project, go to **Project properties -> C/C++ General -> Formatter** page and switch **"Code Formatter"** from **[built-in] **to **"CppStyle (clang-format)"**

## How to use CppStyle

### Style Check with cpplint.py
By pressing the  **Run C/C++ Code Analysis** when you right-click a file, cpplint.py will check the file and display differences in the editor.
In the Properties of CppStyle an automatic analysis can be set to trigger when saving the file.

### Formatting Code with clang-format
The whole file or marked code can be formated using the shortcut **Command + Shift + f** on MacOS or **Ctrl + Shift + f** on Linux and other systems.

Further information: https://github.com/wangzw/CppStyle, http://www.cppstyle.com/
 
