# OMNeT++ Simulator with the following extensions:
# - INET Framework
#
# To be done:
# - simuLTE
# - VEINS
#
# To be discussed: Split this in two containers: 
#   1) omnetpp-base (command line, no GUI, for simulation campaigns)
#   2) omnetpp-ide (all graphical libs, autostart of IDE)


FROM ubuntu:16.04

# Specify the version/tag of omnetpp to be put in the container:
# ENV OMNETRELEASE omnetpp-5.4.1
ENV OMNETRELEASE omnetpp-5.1.1


LABEL description "OMNeT++ Simulation Framework Version 5.1.1"
LABEL maintainer  "wischhof@ieee.org"

# install all packages required to compile omnet
# Note: we currently use jdk-8 due to compatibility problems of omnet with jdk-11
#       (when these are fixed, openjdk-8-jre should be replaced by default-jre)
RUN apt-get update && apt-get install -y curl && curl -s https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh | bash
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
       sudo \
       git git-lfs \ 
       uncrustify nemiver \
       g++ bison flex make qt5-default \
       openjdk-8-jre \
       doxygen graphviz tcl-dev tk-dev python python3 \
       libopenmpi-dev libxml2-dev libwebkitgtk-1.0-0 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# for read access to the (non-public) roVer repos, we need to copy the key of the rover-readonly user:
COPY id_rsa /root/.ssh/id_rsa
RUN chmod 400 /root/.ssh/id_rsa

# clone the OMNeT++ sources (version/tag is specified in the git command at the end)
WORKDIR /opt/omnetpp
# enable lfs (must be done before cloning!) and clone the project
RUN git lfs install && \ 
    GIT_SSH_COMMAND="ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no" git \
        clone --branch $OMNETRELEASE ssh://git@sam-dev.cs.hm.edu:5022/rover/omnetpp.git $OMNETRELEASE


# add a symbolic link so omnetpp is found in the path
RUN ln -s /opt/omnetpp/$OMNETRELEASE /opt/omnetpp/omnetpp

# configure and compile
WORKDIR /opt/omnetpp/omnetpp
# samples are optional and not needed for compiling - remove for a smaller image
# RUN rm -rf samples
ENV PATH /opt/omnetpp/omnetpp/bin:$PATH
COPY configure.user .
RUN ./configure && \
    make -j $(nproc) && \
    find . -type f -name '*.o' -exec rm {} +

# now install the Eclipse IDE base
ADD ide/ide.tar.gz .
RUN chmod -R a+rw /opt/omnetpp/omnetpp/samples && \
    chmod -R a+rw /opt/omnetpp/omnetpp/ide

# TODO: clean up all files not needed anymore (object files, etc.)

# init script (triggered at container start)
COPY init.sh /init.sh
RUN chmod 555 /init.sh
ENTRYPOINT [ "/init.sh" ]

# Note: in order to launch OMNeT with X11 output, the container needs to be launched with:
# docker run -it \
# --user $(id -u) \
# -e DISPLAY=unix$DISPLAY \
# --workdir=$(pwd) \
# --volume="/home/$USER:/home/$USER" \
# --volume="/etc/group:/etc/group:ro" \
# --volume="/etc/passwd:/etc/passwd:ro" \
# --volume="/etc/shadow:/etc/shadow:ro" \
# --volume="/etc/sudoers.d:/etc/sudoers.d:ro" \
# -v /tmp/.X11-unix:/tmp/.X11-unix \
#
# Note: Attaching a debugger to a running simulation requires SYS_PTRACE capabilities:
#       - run with option --cap-add=SYS_PTRACE
#       - docker compose with   cap_add: - SYS_PTRACE


