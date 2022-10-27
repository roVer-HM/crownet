#!/bin/bash
#
# This script upates all docker images required for the roVer project.

docker image pull ubuntu
docker image pull ubuntu:20.04

docker login sam-dev.cs.hm.edu:5023
docker image pull sam-dev.cs.hm.edu:5023/rover/crownet/omnetpp
docker image pull sam-dev.cs.hm.edu:5023/rover/crownet/omnetpp-ide
docker image pull sam-dev.cs.hm.edu:5023/rover/crownet/sumo
docker image pull sam-dev.cs.hm.edu:5023/rover/crownet/vadere
docker image pull sam-dev.cs.hm.edu:5023/rover/crownet/vadere-ide
docker image pull sam-dev.cs.hm.edu:5023/rover/crownet/flowcontrol
docker image pull sam-dev.cs.hm.edu:5023/rover/crownet/flowcontrol-ide

