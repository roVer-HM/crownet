# syntax = docker/dockerfile:experimental
#
# Scope: 
# build the OMNeT++ Simulator based on our public omnetpp mirror >https://github.com/roVer-HM/omnetpp.git<. 
# Note that the OMNeT++ IDE is not included. To build the IDE, go to ../omnetpp-ide/Dockerfile.

# Requirements/dependencies:
# 1. the crownet-base-cpp image must be available (see >FROM ..<). Use ARG to specify the exact name of the crownet-base-cpp image. Go to ../crownet-base-min/Dockerfile

# Release control (how to specify an omnetpp release):
# Go to >https://github.com/roVer-HM/omnetpp.git<. Choose a tag that refers to the desired omnetpp release.
# Then, change >ENV GITHUB_TAG="tag-of-desired-omnetpp-release"< in THIS Dockerfile.
# Always use tags instead of branches so that the code can be uniquely assigned to a unique state.

# Common problems:
# Cloning of the repo fails -> Make sure that the tag is available on >https://github.com/roVer-HM/omnetpp.git<.

ARG IMAGE_REG=invalid
ARG BASE_IMAGE_TAG=latest

FROM ${IMAGE_REG}/crownet-base-cpp:${BASE_IMAGE_TAG}

ENV GITHUB_TAG="omnetpp-6.1.0"
ENV RELEASE=$GITHUB_TAG

ENV OSGEARTH_TAG="osgearth-3.7.2"

ENV ASN1C_HASH="9925dbbda86b436896108439ea3e0a31280a6065"

LABEL description="OMNeT++ Simulation Framework (as used in CrowNet, tag omnetpp-6.1.0)"
LABEL maintainer="wischhof@ieee.org"

# Use bash shell
SHELL ["/bin/bash", "-c"]

# setup image to use bash also as default shell
RUN rm /bin/sh && ln -s /bin/bash /bin/sh

# 
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
        htop libcurl4-gnutls-dev libgl-dev libglfw3-dev libcurl4-gnutls-dev \
        zipcmp libzip-dev zipmerge ziptool \
        libopenscenegraph-dev libopenthreads-dev libgdal-dev libglew-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install the ASN1C Compiler (Version 0.9.29)
WORKDIR /opt
RUN git clone https://github.com/vlm/asn1c.git && \
    cd asn1c && git checkout $ASN1C_HASH
RUN cd asn1c && test -f configure || autoreconf -iv && \
    ./configure && \
    make install

# Build and install osgearth (required for geocoding in CrowNet)
WORKDIR /opt/osgearth
RUN git clone --depth 1 --branch $OSGEARTH_TAG \ 
    --recurse-submodules https://github.com/gwaldron/osgearth.git $OSGEARTH_TAG
RUN ln -s /opt/osgearth/$OSGEARTH_TAG /opt/osgearth/osgearth
WORKDIR /opt/osgearth/$OSGEARTH_TAG
RUN mkdir build && \
    cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=/usr .. && \
    make -j $(nproc) && \
    make install && \
    ldconfig

# clone the OMNeT++ sources (version/tag is specified in the git command at the end)
WORKDIR /opt/omnetpp
# enable lfs (must be done before cloning!) and clone the project
RUN git lfs install

# invalidate cached part by specifying a --build-arg NOCACHE_*=[random value]
ARG NOCACHE_CLONE=0

RUN git clone --depth 1 --branch $GITHUB_TAG https://github.com/roVer-HM/omnetpp.git $GITHUB_TAG

# add a symbolic link so omnetpp is found in the path
RUN ln -s /opt/omnetpp/$GITHUB_TAG /opt/omnetpp/omnetpp

# pull again (so a cached layer having the clone gets updated)
WORKDIR /opt/omnetpp/omnetpp

ARG NOCACHE_PULL=0
RUN git pull https://github.com/roVer-HM/omnetpp.git $GITHUB_TAG

# install, configure and compile
ENV PATH /opt/omnetpp/omnetpp/bin:$PATH
COPY docker/omnetpp/configure.user .
RUN  apt update && ./install.sh -y && source setenv -f && ./configure && \
     make -j $(nproc) && \
     find . -type f -name '*.o' -exec rm {} + && \
     find . -type f -name '*.o.d' -exec rm {} +

# update permissions so that the omnetpp directory becomes writable
RUN chmod -R a+rw /opt/omnetpp/omnetpp

# init script (triggered at container start)
COPY docker/omnetpp/init.sh /init.sh
RUN chmod 555 /init.sh
ENTRYPOINT [ "/init.sh" ]

# waitfor script (in order to wait for non-child processes)
COPY docker/omnetpp/waitfor.sh /waitfor.sh
RUN chmod 555 /waitfor.sh
