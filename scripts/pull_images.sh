#!/bin/bash
#
# This script upates all docker images required for the CrowNet project.

# define version tags
TAG_UBUNTU="20.04"
TAG_OMNETPP="6.0.1"
TAG_SUMO="v1_15_0"
TAG_VADERE="221108-1613"
TAG_CONTROL="220527-1453"

docker image pull ubuntu
docker image pull ubuntu:$TAG_UBUNTU

if [ -z "$CROWNET_IMAGE_BASE" ]; then
      echo "Variabel 'CROWNET_IMAGE_BASE' not set. Did you source $CROWNET_HOME/scrips/crownetenv?"
      exit -1
fi
if [ -z "$CROWNET_IMAGE_REG" ]; then
      echo "Variabel 'CROWNET_IMAGE_REG' not set. Did you source $CROWNET_HOME/scrips/crownetenv?"
      exit -1
fi

IMAGENAME=$CROWNET_IMAGE_BASE/$CONTNAME
echo "access image repo: $CROWNET_IMAGE_REG"
docker login "$CROWNET_IMAGE_REG"

docker image pull "${CROWNET_IMAGE_BASE}/omnetpp"
docker image pull "${CROWNET_IMAGE_BASE}/omnetpp:${TAG_OMNETPP}"
docker image pull "${CROWNET_IMAGE_BASE}/omnetpp-ide"

docker image pull "${CROWNET_IMAGE_BASE}/sumo"
docker image pull "${CROWNET_IMAGE_BASE}/sumo:${TAG_SUMO}"

docker image pull "${CROWNET_IMAGE_BASE}/vadere"
docker image pull "${CROWNET_IMAGE_BASE}/vadere:${TAG_VADERE}"
docker image pull "${CROWNET_IMAGE_BASE}/vadere-ide"

docker image pull "${CROWNET_IMAGE_BASE}/flowcontrol"
docker image pull "${CROWNET_IMAGE_BASE}/flowcontrol-ide"

