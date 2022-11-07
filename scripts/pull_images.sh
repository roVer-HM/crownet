#!/bin/bash
#
# This script upates all docker images required for the CrowNet project.

## define container registry to be used:
# private (internal) CrownNet registry
REGISTRY="sam-dev.cs.hm.edu:5023/rover/crownet"
# public CrowNet registry on github
# REGISTRY="ghcr.io/rover-hm"

# define version tags
TAG_UBUNTU="20.04"
TAG_OMNETPP="6.0.1"
TAG_SUMO="v1_14_1"
TAG_VADERE="220826-1432"
TAG_CONTROL="220527-1453"

docker image pull ubuntu
docker image pull ubuntu:$TAG_UBUNTU

docker login $REGISTRY

docker image pull $REGISTRY/omnetpp
docker image pull $REGISTRY/omnetpp:$TAG_OMNETPP
docker image pull $REGISTRY/omnetpp-ide

docker image pull $REGISTRY/sumo
docker image pull $REGISTRY/sumo:$TAG_SUMO

docker image pull $REGISTRY/vadere
docker image pull $REGISTRY/vadere:$TAG_VADERE
docker image pull $REGISTRY/vadere-ide

docker image pull $REGISTRY/flowcontrol
docker image pull $REGISTRY/flowcontrol:$TAG_CONTROL
docker image pull $REGISTRY/flowcontrol-ide

