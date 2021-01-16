# vadere Crowd Simulator
#
# see https://gitlab.lrz.de/vadere/vadere
#
# TODO:
# - include drivers for HW-accelerated OpenCL/nvidia
# - enable tests (currently tests are sklipped since they fail if no OpenCL device is available)


FROM sam-dev.cs.hm.edu:5023/rover/crownet/rover-base-min:latest

# Specify the version/tag of vadere to be put in the container:
ENV RELEASE master

LABEL description "vadere Crowd Simulator (as used in roVer)"
LABEL maintainer  "wischhof@ieee.org"

# install all dependencies
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
       maven \
       clinfo mesa-opencl-icd ocl-icd-opencl-dev \
       liblwjgl-java liblwjgl-java-doc liblwjgl-java-jni && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# clone the vadere sources (version/tag is specified in the git command at the end)
WORKDIR /tmp/vadere
# enable lfs (must be done before cloning!) and clone the project
RUN git lfs install

# invalidate cached part by specifying a --build-arg NOCACHE_*=[random value]
ARG NOCACHE_CLONE=0
RUN git clone --depth 1 --branch $RELEASE https://gitlab.lrz.de/vadere/vadere.git $RELEASE

# add a symbolic link so vadere is found in the path
RUN ln -f -s /tmp/vadere/$RELEASE /tmp/vadere/vadere

# pull again (so a cached layer having the clone gets updated)
WORKDIR /tmp/vadere/vadere

ARG NOCACHE_PULL=0
RUN git pull

# Nasty workaround to download dependencies from m2
# https://stackoverflow.com/q/14694139/12079469
# I just want all third level dependencies of Vadere in the .m2 repo to
# speed up subsequent builds of vadere.
RUN mvn clean package -Dmaven.test.skip

WORKDIR /
RUN rm -rf /tmp/vadere
