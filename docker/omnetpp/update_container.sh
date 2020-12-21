#!/bin/bash
CONTAINER='omnetpp'
#ADDITONAL_VERSION_TAG="latest"
VERSION_TAG="6.0pre9"
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
$DIR/../../scripts/upd_container.sh $CONTAINER $VERSION_TAG $ADDITIONAL_VERSION_TAG

