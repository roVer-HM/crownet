#!/bin/bash
CONTAINER='omnetpp'
#VERSION_TAG="latest"
VERSION_TAG="6.0pre9
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
$DIR/../../scripts/upd_container.sh $CONTAINER $VERSION_TAG

