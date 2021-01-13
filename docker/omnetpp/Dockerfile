# syntax = docker/dockerfile:1.0-experimental
#
# OMNeT++ Simulator (as used within the roVer project)
#
# Within roVer, we use two different containers:
#   1) omnetpp (command line, no GUI, for simulation campaigns)
#   2) omnetpp-ide (all graphical libs, autostart of IDE)
#
# This Dockerfile is used to build the omnetpp base container,
# which is also the base container for omnetpp-ide


FROM sam-dev.cs.hm.edu:5023/rover/crownet/rover-base-cpp:latest

# Specify the version/tag of omnetpp to be put in the container:
ENV OMNETRELEASE omnetpp-6.0pre10
# Note: Remember to update the description below when changing OMNETRELEASE

LABEL description "OMNeT++ Simulation Framework (as used in roVer, tag omnetpp-6.0pre10)"
LABEL maintainer  "wischhof@ieee.org"

# install all packages required to compile omnet
# Note: Since clang does not emit debug info for types within the standard libraries,
#       we must include debug info for the installed libraries (versions must match!)
#       by manually installing the libstdc++6-8-dbg package
# see rover-base-cpp and rover-base-min

# install additionally needed Python modules
RUN pip3 install numpy pandas matplotlib posix_ipc

# clone the OMNeT++ sources (version/tag is specified in the git command at the end)
WORKDIR /opt/omnetpp
# enable lfs (must be done before cloning!) and clone the project
RUN git lfs install

# invalidate cached part by specifying a --build-arg NOCACHE_*=[random value]
ARG NOCACHE_CLONE=0
RUN --mount=type=secret,id=sshkey,dst=/root/.ssh/id_rsa GIT_SSH_COMMAND="ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no" git \
        clone --depth 1 --branch $OMNETRELEASE ssh://git@sam-dev.cs.hm.edu:6000/rover/omnetpp.git $OMNETRELEASE

# add a symbolic link so omnetpp is found in the path
RUN ln -s /opt/omnetpp/$OMNETRELEASE /opt/omnetpp/omnetpp

# pull again (so a cached layer having the clone gets updated)
WORKDIR /opt/omnetpp/omnetpp

ARG NOCACHE_PULL=0
RUN --mount=type=secret,id=sshkey,dst=/root/.ssh/id_rsa GIT_SSH_COMMAND="ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no" git \
            pull ssh://git@sam-dev.cs.hm.edu:6000/rover/omnetpp.git $OMNETRELEASE

# configure and compile
ENV PATH /opt/omnetpp/omnetpp/bin:$PATH
COPY configure.user .
RUN ./configure && \
     make -j $(nproc) && \
     find . -type f -name '*.o' -exec rm {} + && \
     find . -type f -name '*.o.d' -exec rm {} +

# update permissions so that the omnetpp directory becomes writable
RUN chmod -R a+rw /opt/omnetpp/omnetpp

# init script (triggered at container start)
COPY init.sh /init.sh
RUN chmod 555 /init.sh
ENTRYPOINT [ "/init.sh" ]

# waitfor script (in order to wait for non-child processes)
COPY waitfor.sh /waitfor.sh
RUN chmod 555 /waitfor.sh
