#!/bin/bash
#
# This script upates all docker images required for the roVer project.

docker image pull ubuntu
docker image pull ubuntu:18.04

docker image pull sam-dev.cs.hm.edu:5023/rover/rover-main/omnetpp
docker image pull sam-dev.cs.hm.edu:5023/rover/rover-main/sumo
docker image pull sam-dev.cs.hm.edu:5023/rover/rover-main/vadere
