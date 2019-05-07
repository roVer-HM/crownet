# SUMO - Simulation of Urban MObility
#
# see https://github.com/eclipse/sumo
#
# TODO: 
# - TraCI port setup and connection (e.g. for connecting to the OMNeT++ container)


FROM ubuntu:18.04

# Specify the version/tag of vadere to be put in the container:
# ENV RELEASE master
ENV RELEASE v1_2_0

LABEL description "SUMO - Simulation of Urban MObility (as used in roVer)"
LABEL maintainer  "wischhof@ieee.org"

# install all dependencies
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
       sudo \
       git git-lfs \ 
       cmake python libxerces-c-dev libfox-1.6-dev \
       libgl1-mesa-dev libglu1-mesa-dev libgdal-dev libproj-dev libgl2ps-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# for read access to the (non-public) roVer repos, we need to copy the key of the rover-readonly user:
COPY id_rsa /root/.ssh/id_rsa
RUN chmod 400 /root/.ssh/id_rsa

# clone the sources (version/tag is specified in the git command at the end)
WORKDIR /opt/sumo
# enable lfs (must be done before cloning!) and clone the project
RUN git lfs install && \ 
    GIT_SSH_COMMAND="ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no" git \
        clone --recursive --branch $RELEASE ssh://git@sam-dev.cs.hm.edu:5022/rover/sumo.git $RELEASE


# add a symbolic link so sumo is found in the path
RUN ln -s /opt/sumo/$RELEASE /opt/sumo/sumo

# configure and compile
WORKDIR /opt/sumo/sumo
ENV PATH /opt/sumo/sumo/bin:$PATH
RUN mkdir build/cmake-build && cd build/cmake-build && \
    cmake ../.. && \
    make -j16

# clean-up the build directory
WORKDIR /opt/sumo/sumo
RUN rm -rf build/cmake-build

# init script (triggered at container start)
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


