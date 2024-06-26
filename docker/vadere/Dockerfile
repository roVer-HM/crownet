# vadere Crowd Simulator
#
# With this Dockerfile Vadere is packaged based on the state of the crownet submodule Vadere
# note: tests and plug-ins are skipped during the build process

# the following parent container contains maven 3.6.3 and openJDK 11.0.8

ARG IMAGE_REG
ARG BASE_IMAGE_TAG
ARG CROWNET_COMMIT_HASH

FROM ${IMAGE_REG}/crownet-base-java:${BASE_IMAGE_TAG}

ENV RELEASE $CROWNET_COMMIT_HASH

LABEL description "vadere Crowd Simulator (as used in CrowNet)"
LABEL maintainer  "christina_maria.mayr@hm.edu"

# create script to start vadere gui and add it to the path.
COPY docker/vadere/vadere-gui /usr/local/bin/vadere-gui
RUN chmod 555 /usr/local/bin/vadere-gui

# init script (triggered at container start)
COPY docker/vadere/init.sh /init.sh
RUN chmod 555 /init.sh
CMD [ "/init.sh" ]

# clone the vadere sources (version/tag is specified in the git command at the end)
WORKDIR /opt/vadere

# invalidate cached part by specifying a --build-arg NOCACHE_*=[random value]
ARG NOCACHE_CLONE=1

# remove old vadere version
RUN rm -rf /opt/vadere/vadere

# copy vadere from local crownet repository in docker image
# avoid > COPY vadere/ /opt/vadere/vadere < to reduce the risk of copying private files to the container

WORKDIR /opt/vadere/vadere
RUN ls -a

COPY vadere/pom.xml /opt/vadere/vadere/pom.xml 
COPY vadere/VadereAnnotation /opt/vadere/vadere/VadereAnnotation
COPY vadere/VadereManager /opt/vadere/vadere/VadereManager
COPY vadere/VadereSimulator /opt/vadere/vadere/VadereSimulator
COPY vadere/VadereUtils /opt/vadere/vadere/VadereUtils
COPY vadere/VadereGui /opt/vadere/vadere/VadereGui
COPY vadere/VadereMeshing /opt/vadere/vadere/VadereMeshing
COPY vadere/VadereState /opt/vadere/vadere/VadereState
COPY vadere/Tools /opt/vadere/vadere/Tools


# build vadere in docker container

RUN mvn clean
RUN mvn -Dexec.skip -Dmaven.test.skip=true package

# cleanup class files
RUN find . -type f -name '*.class' -exec rm {} +

# add to path - why?
ENV PATH /opt/vadere/vadere/VadereGui/target:$PATH



