# SUMO - Simulation of Urban MObility
#
# see https://github.com/eclipse/sumo

FROM ubuntu:18.04

# Specify the version/tag of sumo to be put in the container:
# ENV RELEASE master
ENV RELEASE v1_0_1

# VEINS release (for checking out the sumo-launchd.py script required for TraCI)
ENV VEINS_RELEASE hm_master

LABEL description "SUMO - Simulation of Urban MObility (as used in roVer)"
LABEL maintainer  "wischhof@ieee.org"

# install all dependencies
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
       sudo net-tools \
       git git-lfs \
       cmake python libxerces-c-dev libfox-1.6-dev \
       libgl1-mesa-dev libglu1-mesa-dev libgdal-dev libproj-dev libgl2ps-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# clone the sources (version/tag is specified in the git command at the end)
WORKDIR /opt/sumo
# enable lfs (must be done before cloning!) and clone the project
RUN git lfs install

# invalidate cached part by specifying a --build-arg NOCACHE_*=[random value]
ARG NOCACHE_CLONE=0
RUN GIT_SSH_COMMAND="ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no" git \
        clone --recursive --branch $RELEASE ssh://git@sam-dev.cs.hm.edu:6000/rover/sumo.git $RELEASE

# add a symbolic link so sumo is found in the path
RUN ln -s /opt/sumo/$RELEASE /opt/sumo/sumo

WORKDIR /opt/sumo/sumo

ARG NOCACHE_PULL=0
RUN GIT_SSH_COMMAND="ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no" git \
        pull ssh://git@sam-dev.cs.hm.edu:6000/rover/sumo.git $RELEASE

# configure and compile
ENV PATH /opt/sumo/sumo/bin:$PATH
RUN mkdir build/cmake-build && cd build/cmake-build && \
    cmake ../.. && \
    make -j16

# clean-up the build directory
WORKDIR /opt/sumo/sumo
RUN rm -rf build/cmake-build

# Next, we need to aquire the python script to lauch sumo for TraCI from the VEINS project
WORKDIR /opt
RUN GIT_SSH_COMMAND="ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no" git \
        clone -n ssh://git@sam-dev.cs.hm.edu:6000/rover/veins.git --depth 1 && \
    cd veins && \
    git checkout $VEINS_RELEASE sumo-launchd.py && \
    chmod 555 sumo-launchd.py && \
    rm -rf .git

# default init script (triggered at container start)
COPY init.sh /init.sh
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


