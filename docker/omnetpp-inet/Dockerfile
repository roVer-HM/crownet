# syntax = docker/dockerfile:experimental
#
# OMNeT++ Simulator with the following extensions:
# - INET Framework    (in /models/inet4)
# - simu5g            (in /models/simu5g)
# - VEINS             (in /models/veins)


# based on the latest omnet container of the crownet project
FROM sam-dev.cs.hm.edu:5023/rover/crownet/omnetpp:6.0.1

# Specify the version/tag of inet to be put in the container:
ENV INETRELEASE hm_master
ENV SIMU5GRELEASE hm_master
ENV VEINSRELEASE hm_master
# Note: Remember to update the description below when changing versions/tags


LABEL description "OMNeT++ Simulation Framework including inet, simu5g and veins models (crownet project)"
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
RUN source setenv && \
    make makefiles && \
    make MODE=release CONFIGNAME=gcc-release -j $(nproc) all && \
    make MODE=debug CONFIGNAME=gcc-debug -j $(nproc) all && \
    find . -type f -name '*.o' -exec rm {} +

#################### SIMU5G ###########################################################################

# clone the sources (version/tag is specified in the git command at the end)
WORKDIR /models

# invalidate cached part by specifying a --build-arg NOCACHE_*=[random value]
ARG NOCACHE_CLONE=0
RUN --mount=type=ssh git \
       clone --branch $SIMU5GRELEASE ssh://git@sam-dev.cs.hm.edu:6000/rover/Simu5G.git

# pull again (so a cached layer having the clone gets updated)
WORKDIR /models/Simu5G

ARG NOCACHE_PULL=0
RUN --mount=type=ssh git \
       pull ssh://git@sam-dev.cs.hm.edu:6000/rover/Simu5G.git $SIMU5GRELEASE

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


