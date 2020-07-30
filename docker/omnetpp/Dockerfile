# OMNeT++ Simulator (as used within the roVer project)
# To be discussed: Split this in two containers:
#   1) omnetpp-base (command line, no GUI, for simulation campaigns)
#   2) omnetpp-ide (all graphical libs, autostart of IDE)


FROM sam-dev.cs.hm.edu:5023/rover/rover-main/rover-base-cpp:latest

# Specify the version/tag of omnetpp to be put in the container:
ENV OMNETRELEASE omnetpp-5.6
ENV ARTERY_BRANCH hm_master
# Note: Remember to update the description below when changing OMNETRELEASE


LABEL description "OMNeT++ Simulation Framework (as used in roVer, tag omnetpp-5.6)"
LABEL maintainer  "wischhof@ieee.org"

# install all packages required to compile omnet
# Note: Since clang does not emit debug info for types within the standard libraries,
#       we must include debug info for the installed libraries (versions must match!)
#       by manually installing the libstdc++6-8-dbg package
# see rover-base-cpp and rover-base-min

# clone the OMNeT++ sources (version/tag is specified in the git command at the end)
WORKDIR /opt/omnetpp
# enable lfs (must be done before cloning!) and clone the project
RUN git lfs install

# invalidate cached part by specifying a --build-arg NOCACHE_*=[random value]
ARG NOCACHE_CLONE=0
RUN GIT_SSH_COMMAND="ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no" git \
        clone --depth 1 --branch $OMNETRELEASE ssh://git@sam-dev.cs.hm.edu:5022/rover/omnetpp.git $OMNETRELEASE

# add a symbolic link so omnetpp is found in the path
RUN ln -s /opt/omnetpp/$OMNETRELEASE /opt/omnetpp/omnetpp

# pull again (so a cached layer having the clone gets updated)
WORKDIR /opt/omnetpp/omnetpp

ARG NOCACHE_PULL=0
RUN GIT_SSH_COMMAND="ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no" git \
            pull ssh://git@sam-dev.cs.hm.edu:5022/rover/omnetpp.git $OMNETRELEASE

# configure and compile
#  - samples are optional and not needed for compiling - remove for a smaller image
# RUN rm -rf samples
ENV PATH /opt/omnetpp/omnetpp/bin:$PATH
COPY configure.user .
RUN ./configure && \
     make -j $(nproc) && \
     find . -type f -name '*.o' -exec rm {} + && \
     find . -type f -name '*.o.d' -exec rm {} +

# now install the Eclipse IDE base
# ADD ide/ide.tar.gz .


# installation of tools for code formatting and checks (cpplint and cppstyle eclipse plugin)
# RUN pip3 install cpplint
# RUN /opt/omnetpp/omnetpp/bin/omnetpp -application org.eclipse.equinox.p2.director -repository http://www.cppstyle.com/mars -installIU org.wangzw.cppstyle.feature.feature.group ; PID=`pidof omnetpp`; while [ -e /proc/$PID ]; do sleep 0.1; done

# update permissions so that the omnetpp directory becomes writable
RUN chmod -R a+rw /opt/omnetpp/omnetpp


# Install artery
WORKDIR /opt/artery
# invalidate cached part by specifying a --build-arg NOCACHE_*=[random value]
ARG NOCACHE_CLONE=0
RUN GIT_SSH_COMMAND="ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no" git \
        clone --recurse-submodules --depth 1 --branch $ARTERY_BRANCH ssh://git@sam-dev.cs.hm.edu:5022/rover/artery.git


# pull again (so a cached layer having the clone gets updated)
WORKDIR /opt/artery/artery

ARG NOCACHE_PULL=0
RUN GIT_SSH_COMMAND="ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no" git \
        pull ssh://git@sam-dev.cs.hm.edu:5022/rover/artery.git $ARTERY_BRANCH

ENV ARTERY_HOME /opt/artery/artery
ENV VANTEZA_HOME /opt/artery/artery/extern/vanetza

# build externals (release)
RUN make  -j $(nproc) all MODE=release
# build vanetz  (debug)
RUN make  -j $(nproc) all MODE=debug

# build artery release
RUN mkdir -p build/Release
WORKDIR /opt/artery/artery/build/Release
RUN cmake -DCMAKE_BUILD_TYPE=Release ../..
RUN make -j $(nproc)
RUN find . -type f -name '*.o' -exec rm {} + && \
    find . -type f -name '*.o.d' -exec rm {} +

WORKDIR /opt/artery/artery

# build artery debug
RUN mkdir -p build/Debug
WORKDIR /opt/artery/artery/build/Debug
RUN cmake -DCMAKE_BUILD_TYPE=Debug ../..
RUN make -j $(nproc)
RUN find . -type f -name '*.o' -exec rm {} + && \
    find . -type f -name '*.o.d' -exec rm {} +

# update permissions so that the omnetpp directory becomes writable
RUN chmod -R a+rw /opt/artery/artery

# init script (triggered at container start)
COPY init.sh /init.sh
RUN chmod 555 /init.sh
ENTRYPOINT [ "/init.sh" ]

# waitfor script (in order to wait for non-child processes)
COPY waitfor.sh /waitfor.sh
RUN chmod 555 /waitfor.sh
