#!/bin/bash
CONTAINER='flowcontrol'
VERSION_TAG='latest'
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

if [ -z "$1" ]
then
      echo "\$1 is empty - branch not specified."
      exit -1
else
      $DIR/../../scripts/upd_container.sh $CONTAINER $VERSION_TAG --build-arg BRANCH=$1
fi

