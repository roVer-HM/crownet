# flowcontrol traci handler
#
# see https://sam-dev.cs.hm.edu/rover/flowcontrol
#

FROM sam-dev.cs.hm.edu:5023/rover/crownet/crownet-base-min:latest

# Specify the version/tag of the flow controller to be put in the container. defaut master
ARG BRANCH

LABEL description "flow controller (as used in roVer)"
LABEL maintainer  "mayr9@hm.edu"

# clone the flow control sources (version/tag is specified in the git command at the end)
WORKDIR /opt/flowcontrol
# enable lfs (must be done before cloning!) and clone the project
RUN git lfs install

# invalidate cached part by specifying a --build-arg NOCACHE_*=[random value]
ARG NOCACHE_CLONE=0
RUN --mount=type=ssh git clone --depth 1 --branch $BRANCH \
    ssh://git@sam-dev.cs.hm.edu:6000/rover/flowcontrol.git $BRANCH

# add a symbolic link so flowcontrol is found in the path
RUN ln -f -s /opt/flowcontrol/$BRANCH /opt/flowcontrol/flowcontrol

# add a symbolic link to /usr/bin/python3 from /usr/bin/python3.7
RUN ln -f -s /usr/bin/python3.8 /usr/bin/python3

# pull again (so a cached layer having the clone gets updated)
WORKDIR /opt/flowcontrol/flowcontrol

ARG NOCACHE_PULL=0
RUN --mount=type=ssh git \
            pull ssh://git@sam-dev.cs.hm.edu:6000/rover/flowcontrol.git $BRANCH

# configure and install

RUN python3 setup.py install

RUN python3 -m venv /opt/local_venv
RUN . /opt/local_venv/bin/activate && pip install -r requirements.txt
RUN chmod -R ugo+w /opt/local_venv


# init script (triggered at container start)
COPY init.sh /init.sh
COPY init_dev.sh /init_dev.sh
RUN chmod 555 /init.sh
RUN chmod 555 /init_dev.sh
CMD [ "/init.sh" ]
