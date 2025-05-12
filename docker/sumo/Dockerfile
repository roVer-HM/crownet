# syntax = docker/dockerfile:experimental
#
# Scope:
# build the SUMO Simulator based on our public sumo mirror >https://github.com/roVer-HM/sumo.git<.

# Requirements/dependencies:
# none

# Release control (how to specify a SUMO release):
# Go to >https://github.com/roVer-HM/sumo<. Choose a tag that refers to the desired SUMO release.
# Then, change >ENV GITHUB_TAG="tag-of-desired-sumo-release"< in THIS Dockerfile.
# Always use tags instead of branches so that the code can be uniquely assigned to a unique state.

# Common problems:
# Cloning of the repo fails -> Make sure that the tag is available on >https://github.com/roVer-HM/sumo<.

FROM ubuntu:24.04

ENV GITHUB_TAG="v1_23_1_hm"
ENV SUMO_RELEASE=$GITHUB_TAG

# VEINS release (for checking out the sumo-launchd.py script required for TraCI)
ENV VEINS_RELEASE hm_master

LABEL description "SUMO - Simulation of Urban MObility (as used in CrowNet)"
LABEL maintainer  "wischhof@ieee.org"

# install all dependencies
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
       sudo net-tools \
       openssh-client git git-lfs \
       cmake python3 python3-dev g++ libxerces-c-dev libfox-1.6-dev \
       libgl1-mesa-dev libglu1-mesa-dev libgdal-dev libproj-dev libgl2ps-dev \ 
       swig default-jdk maven libeigen3-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# clone the sources (version/tag is specified in the git command at the end)
WORKDIR /opt/sumo
# enable lfs (must be done before cloning!) and clone the project
RUN git lfs install

# invalidate cached part by specifying a --build-arg NOCACHE_*=[random value]
ARG NOCACHE_CLONE=0
RUN git clone --recursive --branch $SUMO_RELEASE https://github.com/roVer-HM/sumo.git $SUMO_RELEASE

# add a symbolic link so sumo is found in the path
RUN ln -s /opt/sumo/$SUMO_RELEASE /opt/sumo/sumo

WORKDIR /opt/sumo/sumo

ARG NOCACHE_PULL=0
RUN git pull https://github.com/roVer-HM/sumo.git $SUMO_RELEASE

# configure and compile
ENV PATH /opt/sumo/sumo/bin:$PATH
ENV SUMO_HOME $PWD
RUN cmake -B build . && \
    cmake --build build -j$(nproc)

# clean-up the build directory
WORKDIR /opt/sumo/sumo
RUN rm -rf build


COPY docker/sumo/veins_launchd /veins_launchd
RUN chmod 555 /veins_launchd

# Setup SUMO_HOME
ENV SUMO_HOME /opt/sumo/sumo

# default init script (triggered at container start)
COPY docker/sumo/init.sh /init.sh
RUN chmod 555 /init.sh
CMD [ "/init.sh" ]

# Note: in order to launch the container with X11 output, the container needs to be launched with:
# docker run -it \
# --user $(id -u) \
# -e DISPLAY=unix$DISPLAY \
# --workdir=$(pwd) \
# --volume="/home/$USER:/home/$USER" \
# --volume="/etc/group:/etc/group:ro" \
# --volume="/etc/passwd:/etc/passwd:ro" \
# --volume="/etc/shadow:/etc/shadow:ro" \
# --volume="/etc/sudoers.d:/etc/sudoers.d:ro" \
# -v /tmp/.X11-unix:/tmp/.X11-unix \


