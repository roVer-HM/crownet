# roVer base container
#

FROM ubuntu:18.04

LABEL description "roVer base container"
LABEL maintainer "stefan.schuhbaeck@hm.edu"


# install all packages required
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
       sudo \
       openssh-client \
       git git-lfs \
       python3.7 python python3 python3-pip\
       curl wget \
       vim \
       openjdk-11-jre \
       linux-tools-common linux-tools-generic linux-cloud-tools-generic && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# download public key for sam-dev.cs.hm.edu
RUN mkdir -p -m 0600 ~/.ssh && ssh-keyscan -p 6000 sam-dev.cs.hm.edu >> ~/.ssh/known_hosts
