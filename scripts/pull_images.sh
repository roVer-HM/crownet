#!/bin/bash
#
# This script upates all docker images required for the CrowNet project.

# define version tags
TAG_UBUNTU="20.04"
# Container tag defined 
# TAG_OMNETPP="6.0.1"
# TAG_SUMO="v1_15_0"
# TAG_VADERE="221108-1613"
# TAG_CONTROL="220527-1453"

TAG_ERR="Container tag variable not set."
[[ -z "$CROWNET_OPP_CONT_TAG" ]] && echo "Omnet $TAG_ERR" && exit -1
[[ -z "$CROWNET_SUMO_CONT_TAG" ]] && echo "Sumo $TAG_ERR" && exit -1
[[ -z "$CROWNET_VADERE_CONT_TAG" ]] && echo "Vadere $TAG_ERR" && exit -1
[[ -z "$CROWNET_CONTROL_CONT_TAG" ]] && echo "Control $TAG_ERR" && exit -1

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
docker image pull "${CROWNET_IMAGE_BASE}/omnetpp:${CROWNET_OPP_CONT_TAG}"
docker image pull "${CROWNET_IMAGE_BASE}/omnetpp-ide"

docker image pull "${CROWNET_IMAGE_BASE}/sumo"
docker image pull "${CROWNET_IMAGE_BASE}/sumo:${CROWNET_SUMO_CONT_TAG}"

docker image pull "${CROWNET_IMAGE_BASE}/vadere" 
docker image pull "${CROWNET_IMAGE_BASE}/vadere:${CROWNET_VADERE_CONT_TAG}" 
docker image pull "${CROWNET_IMAGE_BASE}/vadere-ide"

docker image pull "${CROWNET_IMAGE_BASE}/flowcontrol"
docker image pull "${CROWNET_IMAGE_BASE}/flowcontrol:${CROWNET_CONTROL_CONT_TAG}"
docker image pull "${CROWNET_IMAGE_BASE}/flowcontrol-ide"

