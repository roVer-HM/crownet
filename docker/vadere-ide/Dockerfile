# vadere Crowd Simulator
#
# see https://gitlab.lrz.de/vadere/vadere
#
# TODO:
# - include drivers for HW-accelerated OpenCL/nvidia
# - enable tests (currently tests are sklipped since they fail if no OpenCL device is available)


FROM sam-dev.cs.hm.edu:5023/rover/rover-main/vadere:latest

# Specify the version/tag of vadere to be put in the container:
ENV RELEASE master

LABEL description "vadere Crowd Simulator (as used in roVer)"
LABEL maintainer  "wischhof@ieee.org"

# add ide (intellij)
WORKDIR /opt/intellij
ADD ide/ideaIU-2019.3.3.tar.gz .
ENV PATH="/opt/intellij/idea-IU-193.6494.35/bin:${PATH}"

# init script (triggered at container start)
COPY init.sh /init.sh
RUN chmod 555 /init.sh
ENTRYPOINT [ "/init.sh" ]

# waitfor script (in order to wait for non-child processes)
COPY waitfor.sh /waitfor.sh
RUN chmod 555 /waitfor.sh
