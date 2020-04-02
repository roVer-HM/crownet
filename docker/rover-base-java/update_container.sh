#!/bin/bash
RANDOM=$(date +%s)
IMAGE='sam-dev.cs.hm.edu:5023/rover/rover-main/rover-base-java'
DATE_TAG="$(date "+%y%m%d-%H%M")"
docker build -t "$IMAGE:latest" -t "$IMAGE:$DATE_TAG" --build-arg NOCACHE_PULL=$RANDOM .

if [ $? -eq 0 ]; then
   docker login sam-dev.cs.hm.edu:5023
   docker push "$IMAGE:latest"
   docker push "$IMAGE:$DATE_TAG"
else
   echo "Container build did not succeed - no upload to registry."
fi
