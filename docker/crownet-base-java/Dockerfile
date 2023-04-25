# Java base container with vadere Crowd Simulator
# scope: provide Java and Maven for building and executing Vadere
# note: Vadere requires openJDK11 (it is not tested for newer versions). Therefore, we set maven=3.6.3 and openJDK=11.

ARG IMAGE_REG
ARG CROWNET_COMMIT_HASH
ARG BASE_IMAGE_TAG

FROM ${IMAGE_REG}/crownet-base-min:${BASE_IMAGE_TAG}
ENV RELEASE $CROWNET_COMMIT_HASH

LABEL description "system requirements for vadere Crowd Simulator (as used in crownet)"
LABEL maintainer  "christina_maria.mayr@hm.edu"

RUN apt-get install -y wget 
WORKDIR /opt/vadere/

# install specific java version
RUN wget https://download.java.net/java/ga/jdk11/openjdk-11_linux-x64_bin.tar.gz -P /opt/vadere/
RUN tar xzvf /opt/vadere/openjdk-11_linux-x64_bin.tar.gz
ENV PATH /opt/vadere/jdk-11/bin:$PATH

# install specific maven version
RUN wget https://repo.maven.apache.org/maven2/org/apache/maven/apache-maven/3.6.3/apache-maven-3.6.3-bin.tar.gz -P /opt/vadere/
RUN tar xzvf /opt/vadere/apache-maven-3.6.3-bin.tar.gz
ENV PATH /opt/vadere/apache-maven-3.6.3/bin:$PATH




