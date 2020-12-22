# roVer base container
#

FROM ubuntu:18.04

LABEL description "roVer base container"
LABEL maintainer "stefan.schuhbaeck@hm.edu"


# install all packages required
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
       sudo \
       git git-lfs \
       python3.7 python python3 python3-pip\
       curl wget \
       vim \
       openjdk-11-jre \
       linux-tools-common linux-tools-generic linux-cloud-tools-generic && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

