# vadere Crowd Simulator
#
# see https://gitlab.lrz.de/vadere/vadere
#
# TODO:
# - include drivers for HW-accelerated OpenCL/nvidia
# - enable tests (currently tests are sklipped since they fail if no OpenCL device is available)

ARG IMAGE_REG
ARG BASE_IMAGE_TAG
ARG CROWNET_COMMIT_HASH


FROM ${IMAGE_REG}/flowcontrol:${BASE_IMAGE_TAG}

# Specify the version/tag of flowcontrol to be put in the container:
ENV RELEASE $CROWNET_COMMIT_HASH

LABEL description "FlowControl Simulator (as used in crownet)"
LABEL maintainer  "christina_maria.mayr@hm.edu"

# init script (triggered at container start)
COPY docker/flowcontrol-ide/init.sh /init.sh
RUN chmod 555 /init.sh
ENTRYPOINT [ "/init.sh" ]

# waitfor script (in order to wait for non-child processes)
COPY docker/flowcontrol-ide/waitfor.sh /waitfor.sh
RUN chmod 555 /waitfor.sh

# add ide (pycharm)
WORKDIR /opt/pycharm
RUN wget https://download.jetbrains.com/python/pycharm-professional-2021.1.1.tar.gz --output-document /opt/pycharm/pycharm.tar.gz
RUN tar xzvf /opt/pycharm/pycharm.tar.gz --one-top-level=pycharm --strip-components 1
ENV PATH="/opt/pycharm/pycharm/bin:${PATH}"


