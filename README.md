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
./install_docker.sh
```

Pull the required Docker images (due to the large size of some of them, this will take some time - depending on your Internet connection):
```
./pull_images.sh
```

Now you have a completely installed simulation system. Simply start the container which you want to use, e.g. by:
```
./omnetpp
```
or
```
./vadere
```

Note: The start script will mount your home directory so that it is visible inside the Docker container.
