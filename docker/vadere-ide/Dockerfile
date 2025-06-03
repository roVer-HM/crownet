# vadere Crowd Simulator
#

ARG IMAGE_REG=invalid
ARG BASE_IMAGE_TAG=latest

FROM ${IMAGE_REG}/vadere:${BASE_IMAGE_TAG}

ARG CROWNET_COMMIT_HASH

# Specify the version/tag of vadere to be put in the container:
ENV RELEASE=$CROWNET_COMMIT_HASH

LABEL description="vadere Crowd Simulator with IDE (as used in CrowNet)"
LABEL maintainer="wischhof@ieee.org"

# init script (triggered at container start)
COPY docker/vadere-ide/init.sh /init.sh
RUN chmod 555 /init.sh
ENTRYPOINT [ "/init.sh" ]

# waitfor script (in order to wait for non-child processes)
COPY docker/vadere-ide/waitfor.sh /waitfor.sh
RUN chmod 555 /waitfor.sh

# add ide (intellij)

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
       software-properties-common 

RUN add-apt-repository ppa:mmk2410/intellij-idea && \
    apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
       intellij-idea-community  && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

