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
       openjdk-11-jre \
       linux-tools-common linux-tools-generic linux-cloud-tools-generic && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# for read access to the (non-public) roVer repos, we need to copy the key of the rover-readonly user:
COPY id_rsa /root/.ssh/id_rsa
RUN chmod 400 /root/.ssh/id_rsa
