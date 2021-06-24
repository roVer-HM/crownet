# vadere Crowd Simulator
#
# see https://gitlab.lrz.de/vadere/vadere
#
# TODO:
# - include drivers for HW-accelerated OpenCL/nvidia
# - enable tests (currently tests are sklipped since they fail if no OpenCL device is available)


FROM sam-dev.cs.hm.edu:5023/rover/crownet/flowcontrol:latest

# Specify the version/tag of flowcontrol to be put in the container:
ENV RELEASE master

LABEL description "FlowControl Simulator (as used in roVer)"
LABEL maintainer  "christina_maria.mayr@hm.edu"

# add ide (pycharm)
WORKDIR /opt/pycharm
ADD ~/.cache/crownet/pycharm.tar.gz .
ENV PATH="/opt/pycharm/pycharm-2021.1.1/bin:${PATH}"

# init script (triggered at container start)
COPY init.sh /init.sh
RUN chmod 555 /init.sh
ENTRYPOINT [ "/init.sh" ]

# waitfor script (in order to wait for non-child processes)
COPY waitfor.sh /waitfor.sh
RUN chmod 555 /waitfor.sh
