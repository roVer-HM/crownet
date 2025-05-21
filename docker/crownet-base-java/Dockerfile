# Java base container with vadere Crowd Simulator
# scope: provide Java and Maven for building and executing Vadere

ARG IMAGE_REG=invalid
ARG BASE_IMAGE_TAG=latest

FROM ${IMAGE_REG}/crownet-base-min:${BASE_IMAGE_TAG}

ARG CROWNET_COMMIT_HASH=unknown
ENV RELEASE=${CROWNET_COMMIT_HASH}

LABEL description="CrowNet Java Base container. System requirements for vadere Crowd Simulator (as used in CrowNet)"
LABEL maintainer="wischhof@ieee.org"

RUN echo "Building CrowNet Java base container (${CROWNET_COMMIT_HASH})."

RUN apt-get update && apt-get install -y wget 
WORKDIR /opt/vadere/

# install specific java version
## Java 24
# RUN wget https://download.java.net/java/GA/jdk24.0.1/24a58e0e276943138bf3e963e6291ac2/9/GPL/openjdk-24.0.1_linux-x64_bin.tar.gz -P /opt/vadere/
# RUN tar xzvf /opt/vadere/openjdk-24.0.1_linux-x64_bin.tar.gz
# ENV PATH=/opt/vadere/jdk-24.0.1/bin:$PATH

## Java 21  (default for Ubuntu 24.04.2)
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
       openjdk-21-jdk && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# install specific maven version
RUN wget https://dlcdn.apache.org/maven/maven-3/3.9.9/binaries/apache-maven-3.9.9-bin.tar.gz -P /opt/vadere/ 
RUN tar xzvf /opt/vadere/apache-maven-3.9.9-bin.tar.gz
ENV PATH=/opt/vadere/apache-maven-3.9.9/bin:$PATH




