# vadere Crowd Simulator
#
# see https://gitlab.lrz.de/vadere/vadere
#
# TODO:
# - include drivers for HW-accelerated OpenCL/nvidia
# - enable tests (currently tests are sklipped since they fail if no OpenCL device is available)



FROM arm64v8/ubuntu:20.04
# install all packages required
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    sudo \
    openssh-client \
    git git-lfs \
    python3.8 python3.7 python python3 python3-pip \
    python3.8-venv python3-venv python3.8-tk python3-tk \
    curl wget \
    vim \
    openjdk-11-jre \
    linux-tools-common linux-tools-generic && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*
# Specify the version/tag of vadere to be put in the container:
ENV RELEASE master

# install all dependencies
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    maven \
    clinfo mesa-opencl-icd ocl-icd-opencl-dev \
    liblwjgl-java liblwjgl-java-doc liblwjgl-java-jni && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# clone the vadere sources (version/tag is specified in the git command at the end)
WORKDIR /tmp/vadere
# enable lfs (must be done before cloning!) and clone the project
RUN git lfs install

# invalidate cached part by specifying a --build-arg NOCACHE_*=[random value]
ARG NOCACHE_CLONE=0
RUN git clone --depth 1 --branch $RELEASE https://github.com/learup/vadere.git $RELEASE

# add a symbolic link so vadere is found in the path
RUN ln -f -s /tmp/vadere/$RELEASE /tmp/vadere/vadere

# pull again (so a cached layer having the clone gets updated)
WORKDIR /tmp/vadere/vadere

ARG NOCACHE_PULL=0
RUN git pull

# Nasty workaround to download dependencies from m2
# https://stackoverflow.com/q/14694139/12079469
# I just want all third level dependencies of Vadere in the .m2 repo to
# speed up subsequent builds of vadere.
RUN mvn clean package -Dmaven.test.skip

WORKDIR /
RUN rm -rf /tmp/vadere


# Specify the version/tag of vadere to be put in the container. defaut master
ARG BRANCH



# clone the vadere sources (version/tag is specified in the git command at the end)
WORKDIR /opt/vadere
# enable lfs (must be done before cloning!) and clone the project
RUN git lfs install

# invalidate cached part by specifying a --build-arg NOCACHE_*=[random value]
ARG NOCACHE_CLONE=0
RUN git clone  https://github.com/learup/vadere.git

# add a symbolic link so vadere is found in the path
RUN ln -f -s /opt/vadere/$BRANCH /opt/vadere/vadere

# add a symbolic link to /usr/bin/python3 from /usr/bin/python3.8
RUN ln -f -s /usr/bin/python3.8 /usr/bin/python3

# pull again (so a cached layer having the clone gets updated)
WORKDIR /opt/vadere/vadere

ARG NOCACHE_PULL=0
RUN git pull

# configure and compile
ENV PATH /opt/vadere/vadere/VadereGui/target:$PATH
RUN mvn clean package -Dmaven.test.skip

# cleanup class files
RUN find . -type f -name '*.class' -exec rm {} +

# create script to start vadere gui and add it to the path.
COPY vadere-gui /usr/local/bin/vadere-gui
RUN chmod 555 /usr/local/bin/vadere-gui

# init script (triggered at container start)
COPY init.sh /init.sh
RUN chmod 555 /init.sh
CMD [ "/init.sh" ]
