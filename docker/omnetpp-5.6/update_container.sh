#!/bin/bash
CONTAINER='omnetpp'
VERSION_TAG="5.6.2"
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
$DIR/../../scripts/upd_container.sh $CONTAINER $VERSION_TAG

