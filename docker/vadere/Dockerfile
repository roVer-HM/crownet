# vadere Crowd Simulator
#
# see https://gitlab.lrz.de/vadere/vadere
#
# TODO:
# - include drivers for HW-accelerated OpenCL/nvidia
# - enable tests (currently tests are sklipped since they fail if no OpenCL device is available)


FROM sam-dev.cs.hm.edu:5023/rover/rover-main/rover-base-java:latest

# Specify the version/tag of vadere to be put in the container. defaut master
ARG BRANCH

LABEL description "vadere Crowd Simulator (as used in roVer)"
LABEL maintainer  "wischhof@ieee.org"

# clone the vadere sources (version/tag is specified in the git command at the end)
WORKDIR /opt/vadere
# enable lfs (must be done before cloning!) and clone the project
RUN git lfs install

# invalidate cached part by specifying a --build-arg NOCACHE_*=[random value]
ARG NOCACHE_CLONE=0
RUN git clone --depth 1 --branch $BRANCH https://gitlab.lrz.de/vadere/vadere.git $BRANCH

# add a symbolic link so vadere is found in the path
RUN ln -f -s /opt/vadere/$BRANCH /opt/vadere/vadere

# add a symbolic link to /usr/bin/python3 from /usr/bin/python3.7
RUN ln -f -s /usr/bin/python3.7 /usr/bin/python3

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
