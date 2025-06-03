# CrowNet C++ base container
#

ARG IMAGE_REG=invalid
ARG BASE_IMAGE_TAG=latest

FROM ${IMAGE_REG}/crownet-base-min:${BASE_IMAGE_TAG}

ARG CROWNET_COMMIT_HASH=unknown
ENV RELEASE=${CROWNET_COMMIT_HASH}

LABEL description="CrowNet CPP base container"
LABEL maintainer="wischhof@ieee.org"


RUN echo "Building CrowNet C++ base container (${CROWNET_COMMIT_HASH})."

# Install all packages required for CrowNet containers using g++/gcc
# Note: Since clang does not emit debug info for types within the standard libraries,
#       we must include debug info for the installed libraries (versions must match!)
#       by manually installing the libstdc++6-??-dbg package
#       To check the required version, run g++ --version in the crownet-base-min container.
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
       uncrustify \
       perl zlib1g-dev \
       clang bison flex make qt6-base-dev libqt6opengl6t64 \
       libstdc++6-13-dbg \
       clang-format cpplint \
       bear cmake \
       sqlite3 \
       libsqlite3-dev \
       libboost-all-dev libcrypto++-dev libcrypto++-utils libcrypto++8t64 libgeographiclib-dev doxygen graphviz tcl-dev tk-dev valgrind\
       libopenmpi-dev libxml2-dev \
       python3-numpy python3-pandas python3-matplotlib python3-scipy python3-seaborn && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# download public key for sam-dev.cs.hm.edu
RUN mkdir -p -m 0600 ~/.ssh && ssh-keyscan -p 6000 sam-dev.cs.hm.edu >> ~/.ssh/known_hosts

# install google test library
RUN mkdir download
WORKDIR /download
RUN wget https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
RUN tar -xvf googletest-1.17.0.tar.gz
WORKDIR /download/googletest-1.17.0/
RUN cmake CMakeLists.txt
RUN make install
WORKDIR /
RUN rm -rf download
