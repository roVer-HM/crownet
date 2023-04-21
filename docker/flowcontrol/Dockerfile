# flowcontrol 
#
# With this Dockerfile flowcontrol is installed based on the state of the crownet submodule flowcontrol

ARG IMAGE_REG
ARG BASE_IMAGE_TAG
ARG CROWNET_COMMIT_HASH

FROM ${IMAGE_REG}/crownet-base-min:${BASE_IMAGE_TAG}

ENV RELEASE $CROWNET_COMMIT_HASH

LABEL description "flow controller (as used in roVer)"
LABEL maintainer  "christina_maria.mayr@hm.edu"

# init script (triggered at container start)
COPY docker/flowcontrol/init.sh /init.sh
COPY docker/flowcontrol/init_dev.sh /init_dev.sh
RUN chmod 555 /init.sh
RUN chmod 555 /init_dev.sh
CMD [ "/init.sh" ]


# clone the flow control sources (version/tag is specified in the git command at the end)
WORKDIR /opt/flowcontrol

# invalidate cached part by specifying a --build-arg NOCACHE_*=[random value]
ARG NOCACHE_CLONE=0

# add a symbolic link to /usr/bin/python3 from /usr/bin/python3.7
RUN ln -f -s /usr/bin/python3.8 /usr/bin/python3

# remove old flowcontrol version
RUN rm -rf /opt/flowcontrol/flowcontrol

COPY flowcontrol/flowcontrol /opt/flowcontrol/flowcontrol/flowcontrol 
COPY flowcontrol/bin /opt/flowcontrol/flowcontrol/bin
COPY flowcontrol/MANIFEST.in /opt/flowcontrol/flowcontrol/MANIFEST.in
COPY flowcontrol/requirements.txt /opt/flowcontrol/flowcontrol/requirements.txt 
COPY flowcontrol/LICENSE.txt /opt/flowcontrol/flowcontrol/LICENSE.txt
COPY flowcontrol/setup.py /opt/flowcontrol/flowcontrol/setup.py 

# configure and install

WORKDIR /opt/flowcontrol/flowcontrol
RUN python3 setup.py install

RUN python3 -m venv /opt/local_venv
RUN . /opt/local_venv/bin/activate && pip install -r requirements.txt
RUN chmod -R ugo+w /opt/local_venv


