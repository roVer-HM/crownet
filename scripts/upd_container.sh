#!/bin/bash
# Update a container within the rover container registry.
#
# $1 is the image short name
# $2 is the version tag
#
# All further arguments are passed to the docker build command, e.g. in order to set additional build arguments.
#
if [ -z "$CROWNET_IMAGE_REG" ]; then
      echo "Variabel 'CROWNET_IMAGE_REG' not set. Did you source $CROWNET_HOME/scrips/crownetenv?"
      exit -1
fi

IMAGE_SHORT="$1"
RANDOM=$(date +%s)
REGISTRY=${CROWNET_IMAGE_REG}
IMAGE_LONG="$REGISTRY/rover/crownet/$IMAGE_SHORT"
VERSION_TAG="$2"
DATE_TAG="$(date "+%y%m%d-%H%M")"

export DOCKER_BUILDKIT=1

if [ -z "$IMAGE_SHORT" ]; then
    echo "Illegal number of command line arguments."
    echo "Usage: $0 [container_short_name] [version_tag] [additional arguments to be passed to docker]"
    exit -1
fi

if [ -z "${CROWNET_IMAGE_DEFAULT_TAG}" ]; then
      echo "Variabel 'CROWNET_IMAGE_DEFAULT_TAG' not set. Did you source $CROWNET_HOME/scrips/crownetenv?"
      exit -1
fi
if [ -z "$VERSION_TAG" ]; then
    echo "No version tag specified - using default tag \"${CROWNET_IMAGE_DEFAULT_TAG}\"."
    VERSION_TAG="$CROWNET_IMAGE_DEFAULT_TAG"
fi

TAG_OPTIONS="-t $IMAGE_LONG:$VERSION_TAG -t $IMAGE_LONG:$DATE_TAG -t ghcr.io/rover-hm/$IMAGE_SHORT:$VERSION_TAG -t ghcr.io/rover-hm/$IMAGE_SHORT:$DATE_TAG"

echo "Adding ssh key(s) to ssh agent (might be required for access to private repos)..."
ssh-add

echo "Building $IMAGE_SHORT ..."
docker build $TAG_OPTIONS --ssh default --build-arg NOCACHE_PULL=$RANDOM ${@:3:${#@}+1-3} .

if [ $? -eq 0 ]; then
   docker login "$REGISTRY"
   docker push "$IMAGE_LONG:$VERSION_TAG"
   docker push "$IMAGE_LONG:$DATE_TAG"
else
   echo "Container build did not succeed - $IMAGE_SHORT not uploaded to registry."
fi
