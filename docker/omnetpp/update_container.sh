#!/bin/bash
RANDOM=$(date +%s)
IMAGE='sam-dev.cs.hm.edu:5023/rover/rover-main/omnetpp'
DATE_TAG="$(date "+%y%m%d-%H%M")"
#VERSION_TAG="latest"
VERSION_TAG="6.0pre9"
docker build -t "$IMAGE:$VERSION_TAG" -t "$IMAGE:$DATE_TAG" --build-arg NOCACHE_PULL=$RANDOM .


if [ $? -eq 0 ]; then
   docker login sam-dev.cs.hm.edu:5023
   docker push "$IMAGE:$VERSION_TAG"
   docker push "$IMAGE:$DATE_TAG"
else
   echo "Container build did not succeed - no upload to registry."
fi
