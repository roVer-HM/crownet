# vadere Crowd Simulator
#
# see https://gitlab.lrz.de/vadere/vadere
#
# TODO:
# - include drivers for HW-accelerated OpenCL/nvidia
# - enable tests (currently tests are sklipped since they fail if no OpenCL device is available)

ARG IMAGE_REG=invalid
ARG BASE_IMAGE_TAG=latest

FROM ${IMAGE_REG}/flowcontrol:${BASE_IMAGE_TAG}

ARG CROWNET_COMMIT_HASH

# Specify the version/tag of flowcontrol to be put in the container:
ENV RELEASE=$CROWNET_COMMIT_HASH

LABEL description="FlowControl Simulator (as used in CrowNet)"
LABEL maintainer="wischhof@ieee.org"

# init script (triggered at container start)
COPY docker/flowcontrol-ide/init.sh /init.sh
RUN chmod 555 /init.sh
ENTRYPOINT [ "/init.sh" ]

# waitfor script (in order to wait for non-child processes)
COPY docker/flowcontrol-ide/waitfor.sh /waitfor.sh
RUN chmod 555 /waitfor.sh

# add ide (pycharm)
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
       software-properties-common 

RUN add-apt-repository ppa:xtradeb/apps && \
    apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
       pycharm-community  && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*


