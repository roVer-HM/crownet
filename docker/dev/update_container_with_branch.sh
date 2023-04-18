#!/bin/bash
IMAGE_SHORT='vadere'
VERSION_TAG='latest'
REGISTRY='sam-dev.cs.hm.edu:5023'
IMAGE_LONG="$REGISTRY/rover/crownet/$IMAGE_SHORT"
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

if [ -z "$1" ]
then
      echo "\$1 is empty - branch not specified."
      exit -1
else
      $DIR/../../scripts/upd_container.sh $IMAGE_SHORT $VERSION_TAG -t "$IMAGE_LONG:branch_$1" --build-arg BRANCH=$1
fi

