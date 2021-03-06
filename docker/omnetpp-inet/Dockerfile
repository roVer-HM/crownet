# syntax = docker/dockerfile:experimental
#
# OMNeT++ Simulator with the following extensions:
# - INET Framework    (in /models/inet4)
# - simuLTE           (in /models/simulte)
# - VEINS             (in /models/veins)


# based on the latest omnet container of the rover project
FROM sam-dev.cs.hm.edu:5023/rover/crownet/omnetpp:latest

# Specify the version/tag of inet to be put in the container:
ENV INETRELEASE v4.3.0
ENV SIMULTERELEASE hm_master
ENV VEINSRELEASE hm_master
# Note: Remember to update the description below when changing versions/tags


LABEL description "OMNeT++ Simulation Framework including inet, simulte and veins models (roVer project)"
LABEL maintainer  "wischhof@ieee.org"

#################### INET #############################################################################

# clone the inet sources (version/tag is specified in the git command at the end)
WORKDIR /models
# enable lfs (must be done before cloning!) and clone the project
RUN git lfs install

# invalidate cached part by specifying a --build-arg NOCACHE_*=[random value]
ARG NOCACHE_CLONE=0
RUN --mount=type=ssh git \
        clone --branch $INETRELEASE ssh://git@sam-dev.cs.hm.edu:6000/rover/inet.git inet4

# pull again (so a cached layer having the clone gets updated)
WORKDIR /models/inet4

ARG NOCACHE_PULL=0
RUN --mount=type=ssh git \
        pull ssh://git@sam-dev.cs.hm.edu:6000/rover/inet.git $INETRELEASE

# configure and compile
RUN make makefiles && \
    make MODE=release CONFIGNAME=gcc-release -j $(nproc) all && \
    make MODE=debug CONFIGNAME=gcc-debug -j $(nproc) all && \
    find . -type f -name '*.o' -exec rm {} +

#################### SIMULTE ###########################################################################

# clone the sources (version/tag is specified in the git command at the end)
WORKDIR /models

# invalidate cached part by specifying a --build-arg NOCACHE_*=[random value]
ARG NOCACHE_CLONE=0
RUN --mount=type=ssh git \
       clone --branch $SIMULTERELEASE ssh://git@sam-dev.cs.hm.edu:6000/rover/simulte.git

# pull again (so a cached layer having the clone gets updated)
WORKDIR /models/simulte

ARG NOCACHE_PULL=0
RUN --mount=type=ssh git \
       pull ssh://git@sam-dev.cs.hm.edu:6000/rover/simulte.git $SIMULTERELEASE

# configure and compile
RUN make makefiles && \
    make -j $(nproc) mode=DEBUG && \
    make -j $(nproc) mode=RELEASE && \
    find . -type f -name '*.o' -exec rm {} +

#################### VEINS ###########################################################################

# clone the sources (version/tag is specified in the git command at the end)
WORKDIR /models

# invalidate cached part by specifying a --build-arg NOCACHE_*=[random value]
ARG NOCACHE_CLONE=0
RUN --mount=type=ssh git \
       clone --branch $VEINSRELEASE ssh://git@sam-dev.cs.hm.edu:6000/rover/veins.git

# pull again (so a cached layer having the clone gets updated)
WORKDIR /models/veins

ARG NOCACHE_PULL=0
RUN --mount=type=ssh git \
        pull ssh://git@sam-dev.cs.hm.edu:6000/rover/veins.git $VEINSRELEASE

# configure and compile
RUN ./configure && \
    make -j $(nproc) mode=DEBUG && \
    make -j $(nproc) mode=RELEASE && \
    find . -type f -name '*.o' -exec rm {} +

WORKDIR /models/veins/subprojects/veins_inet
# configure and compile
RUN ./configure --with-inet=/models/inet4 && \
    make -j $(nproc) mode=DEBUG && \
    make -j $(nproc) mode=RELEASE && \
    find . -type f -name '*.o' -exec rm {} +


