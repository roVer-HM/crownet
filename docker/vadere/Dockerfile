# vadere Crowd Simulator
#
# see https://gitlab.lrz.de/vadere/vadere
#
# TODO:
# - include drivers for HW-accelerated OpenCL/nvidia
# - enable tests (currently tests are sklipped since they fail if no OpenCL device is available)


FROM ubuntu:18.04

# Specify the version/tag of vadere to be put in the container:
ENV RELEASE master



LABEL description "vadere Crowd Simulator (as used in roVer)"
LABEL maintainer  "wischhof@ieee.org"

# install all dependencies
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
       sudo \
       git git-lfs \ 
       default-jre \
       maven \
       clinfo mesa-opencl-icd ocl-icd-opencl-dev \
       liblwjgl-java liblwjgl-java-doc liblwjgl-java-jni && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# clone the vadere sources (version/tag is specified in the git command at the end)
WORKDIR /opt/vadere
# enable lfs (must be done before cloning!) and clone the project
RUN git lfs install && \ 
    git clone --branch $RELEASE https://gitlab.lrz.de/vadere/vadere.git $RELEASE


# add a symbolic link so vadere is found in the path
RUN ln -s /opt/vadere/$RELEASE /opt/vadere/vadere

# configure and compile
WORKDIR /opt/vadere/vadere
ENV PATH /opt/vadere/vadere/VadereGui/target:$PATH
RUN mvn clean package -Dmaven.test.skip

# init script (triggered at container start)
COPY init.sh /init.sh
RUN chmod 555 /init.sh
CMD [ "/init.sh" ]

# Note: in order to launch vadere with X11 output, the container needs to be launched with:
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


