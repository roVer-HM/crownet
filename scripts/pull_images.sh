#!/bin/bash
#
# This script upates all docker images required for the roVer project.

docker image pull ubuntu
docker image pull ubuntu:18.04

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
docker image pull "${CROWNET_IMAGE_BASE}/omnetpp-ide"
docker image pull "${CROWNET_IMAGE_BASE}/sumo"
docker image pull "${CROWNET_IMAGE_BASE}/vadere"
docker image pull "${CROWNET_IMAGE_BASE}/vadere-ide"
docker image pull "${CROWNET_IMAGE_BASE}/flowcontrol"
docker image pull "${CROWNET_IMAGE_BASE}/flowcontrol-ide"

