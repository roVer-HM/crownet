

# Scope: 
# provide IDE for the OMNeT++ Simulator based on our public omnetpp mirror >https://github.com/roVer-HM/omnetpp.git<. 

# Requirements/dependencies:
# 1. the omnetpp image must be available (see >FROM ..<). Use ARG to specify the exact name of the crownet-base-cpp image. Go to ../omnetpp/Dockerfile

# Release control (how to specify an omnetpp release):
# The omnetpp release is specified by the base image "omnetpp". To build that, go to ../omnetpp/Dockerfile.
# The release of the IDE is specified through the package release. To change the release, adjust the URL (RUN wget ... /download/omnetpp-6.0.1/omnetpp-6.0.1-linux-x86_64.tgz<)

ARG IMAGE_REG=invalid
ARG BASE_IMAGE_TAG=latest

FROM ${IMAGE_REG}/omnetpp:${BASE_IMAGE_TAG}

ARG CROWNET_COMMIT_HASH
ENV RELEASE=$CROWNET_COMMIT_HASH

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y rsync 

# init script (triggered at container start)
COPY docker/omnetpp-ide/init.sh /init.sh
RUN chmod 555 /init.sh
ENTRYPOINT [ "/init.sh" ]

# waitfor script (in order to wait for non-child processes)
COPY docker/omnetpp-ide/waitfor.sh /waitfor.sh
RUN chmod 555 /waitfor.sh

RUN rm -rf /tmp/omnetpp.tgz
RUN wget https://github.com/omnetpp/omnetpp/releases/download/omnetpp-6.1.0/omnetpp-6.1.0-linux-x86_64.tgz --output-document /tmp/omnetpp.tgz

# extract IDE and copy to omnetpp
WORKDIR /tmp/
RUN tar -xf omnetpp.tgz --one-top-level=omnetpp --strip-components 1

WORKDIR /opt/omnetpp/omnetpp
RUN rm -rf /opt/omnetpp/omnetpp/ide
RUN rsync -aP --chmod=Du=rwx,Dg=rwx,Do=rwx,Fu=rwx,Fg=rwx,Fo=rwx //tmp/omnetpp/ide //opt/omnetpp/omnetpp

# update permissions so that the omnetpp directory becomes writable
#RUN chmod --verbose -R a+rw /opt/omnetpp/omnetpp/ide

# install embeded browser components
RUN apt-get update -y
RUN export SWT_GTK3=1
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libgtk-3-dev libwebkit2gtk-4.1-0
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
       libgtk-3-dev libwebkit2gtk-4.1-0 \
       libcanberra-gtk-module libcanberra-gtk3-module && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# check if PID if is empty. If so this will create an infinte loop at while [ -e /proc/ ]
#RUN /opt/omnetpp/omnetpp/bin/omnetpp -application org.eclipse.equinox.p2.director \
#        -repository http://www.cppstyle.com/mars -installIU org.wangzw.cppstyle.feature.feature.group; \
#        PID=`pidof omnetpp`; \
#        if [ -z "$PID" ]; then echo "error installing plugin. Is ide installed?" && exit 1; fi; \
#        while [ -e /proc/$PID ]; do sleep 0.1 ; done



