#!/bin/bash
RANDOM=$(date +%s)
IMAGE='sam-dev.cs.hm.edu:5023/rover/rover-main/vadere'
DATE_TAG="$(date "+%y%m%d-%H%M")"
docker build -t "$IMAGE:latest" -t "$IMAGE:$DATE_TAG" -t "$IMAGE:branch_$1" --build-arg NOCACHE_PULL=$RANDOM --build-arg BRANCH=$1 .

if [ $? -ne 0 ];then
   echo "Container build did not succeed - no upload to registry."
   exit -1
fi

if [ ! -z $2 ]; then
   echo "docker login sam-dev.cs.hm.edu:5023"
   docker push "$IMAGE:latest"
   docker push "$IMAGE:$DATE_TAG"
   docker push "$IMAGE:branch_$1"
else
   echo "OK - no upload to registry."
fi
