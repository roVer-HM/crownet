# vadere Crowd Simulator
#
# see https://gitlab.lrz.de/vadere/vadere
#
# TODO:
# - include drivers for HW-accelerated OpenCL/nvidia
# - enable tests (currently tests are sklipped since they fail if no OpenCL device is available)


FROM sam-dev.cs.hm.edu:5023/rover/crownet/vadere:latest

# Specify the version/tag of vadere to be put in the container:
ENV RELEASE master

LABEL description "vadere Crowd Simulator (as used in crownet)"
LABEL maintainer  "christina_maria.mayr@hm.edu"

# add ide (intellij)
WORKDIR /opt/intellij
ADD ~/.cache/crownet/intellij.tar.gz .
ENV PATH="/opt/intellij/idea-IU-211.7442.40/bin:${PATH}"
# for community version: replace 'idea-IU-211.7442.40' by 'idea-IC-211.7442.40'

# init script (triggered at container start)
COPY init.sh /init.sh
RUN chmod 555 /init.sh
ENTRYPOINT [ "/init.sh" ]

# waitfor script (in order to wait for non-child processes)
COPY waitfor.sh /waitfor.sh
RUN chmod 555 /waitfor.sh
