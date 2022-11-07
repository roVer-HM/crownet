#!/bin/bash
CONTAINER='sumo'
VERSION_TAG='v1_14_1'
# VERSION_TAG='latest'
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
$DIR/../../scripts/upd_container.sh $CONTAINER $VERSION_TAG

