# crownet base container
#

FROM ubuntu:24.04

ARG CROWNET_COMMIT_HASH=unknown
ENV RELEASE=${CROWNET_COMMIT_HASH}

LABEL description="CrowNet base container"
LABEL maintainer="wischhof@ieee.org"

# setup using mirrors
COPY docker/crownet-base-min/mirrors.txt /etc/apt/
RUN cat /etc/apt/mirrors.txt /etc/apt/sources.list > /etc/apt/temp_sources.list && mv /etc/apt/temp_sources.list /etc/apt/sources.list

# install all packages required
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
       sudo \
       openssh-client \
       git git-lfs \
       python3.12 python3 python3-pip \
       python3.12-venv python3-venv python3.12-tk python3-tk \
       curl wget \
       vim \
       openjdk-11-jre \
       linux-tools-common linux-tools-generic linux-cloud-tools-generic && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# download public key for sam-dev.cs.hm.edu
RUN mkdir -p -m 0600 ~/.ssh && ssh-keyscan -p 6000 sam-dev.cs.hm.edu >> ~/.ssh/known_hosts
